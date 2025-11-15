#include "hw7.h"
#include <string.h>

bst_sf* insert_bst_sf(matrix_sf *mat, bst_sf *root) {
    if(root == NULL) {
        bst_sf *node = malloc(sizeof(bst_sf));
        node->matrix = mat;
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    if(mat->name < root->matrix->name) {
        root->left = insert_bst_sf(mat, root->left);
    } else {
        root->right = insert_bst_sf(mat, root->right);
    }

    return root;
}

matrix_sf* find_bst_sf(char name, bst_sf *root) {
    if(root == NULL) return NULL;

    if(name == root->matrix->name) {
        return root->matrix;
    } else if (name < root->matrix->name) {
        return find_bst_sf(name, root->left);
    } else {
        return find_bst_sf(name, root->right);
    }
}

void free_bst_sf(bst_sf *root) {
    if(root != NULL) {
        free_bst_sf(root->left);
        free_bst_sf(root->right);
        free(root->matrix);
        free(root);
    }
}

matrix_sf* add_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    int rows = mat1->num_rows;
    int cols = mat1->num_cols;

    matrix_sf *result = malloc(sizeof(matrix_sf) + rows*cols * sizeof(int));

    result->num_rows = rows;
    result->num_cols = cols;

    for (int i = 0; i < rows*cols; i++) {
        result->values[i] = mat1->values[i] + mat2->values[i];
    }

    return result;
}

matrix_sf* mult_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    int rows = mat1->num_rows;
    int common = mat1->num_cols;
    int cols = mat2->num_cols;

    matrix_sf *result = malloc(sizeof(matrix_sf) + rows*cols * sizeof(int));

    result->num_rows = rows;
    result->num_cols = cols;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int sum = 0;
            for(int k = 0; k < common; k++) {
                sum += mat1->values[i * common + k] * mat2->values[k * cols + j];
            }
            result->values[i * cols + j] = sum;
        }
    }
    
    return result;
}

matrix_sf* transpose_mat_sf(const matrix_sf *mat) {
    int rows = mat->num_cols;
    int cols = mat->num_rows;

    matrix_sf *result = malloc(sizeof(matrix_sf) + rows*cols * sizeof(int));

    result->num_rows = rows;
    result->num_cols = cols;

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            result->values[i * cols + j] = mat->values[j * rows + i];
        }
    }

    return result;
}

matrix_sf* create_matrix_sf(char name, const char *expr) {
    int rows = 0;
    int cols = 0;
    while(*expr == ' ') expr++;
    while (*expr >= '0' && *expr <= '9') {
        rows = rows * 10 + (*expr - '0');
        expr++;
    }
    while(*expr == ' ') expr++;
    while (*expr >= '0' && *expr <= '9') {
        cols = cols * 10 + (*expr - '0');
        expr++;
    }

    matrix_sf *m = malloc(sizeof(matrix_sf) + rows*cols * sizeof(int));
    m->name = name;
    m->num_rows = rows;
    m->num_cols = cols;

    while (*expr && *expr != '[') expr++;
    if (*expr == '[') expr++;

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            while (*expr == ' ' || *expr == '\n') expr++;

            int sign = 1;
            if (*expr == '-') {
                sign = -1;
                expr++;
            }

            int num = 0;
            while (*expr >= '0' && *expr <= '9') {
                num = num * 10 + (*expr - '0');
                expr++;
            }

            m->values[i * cols + j] = sign * num;
        }

        while (*expr == ' ' || *expr == '\n' || *expr == ';') expr++;
    }

    return m;
}

static int highest(char op) {
    if (op == '\'') return 3;
    if (op == '*')  return 2;
    if (op == '+')  return 1;
    return 0;
}

char* infix2postfix_sf(char *infix) {
    int n = 0;
    while (infix[n] != '\0') n++;

    char *out = malloc(n + 1);
    int out_i = 0;

    char *stack = malloc(n);
    int top = -1;

    for (int i = 0; infix[i] != '\0'; i++) {
        char c = infix[i];

        if (c == ' ') continue;

        if (c >= 'A' && c <= 'Z') {
            out[out_i++] = c;
            continue;
        }

        if (c == '(') {
            stack[++top] = c;
            continue;
        }

        if (c == ')') {
            while (top >= 0 && stack[top] != '(') {
                out[out_i++] = stack[top--];
            }
            if (top >= 0 && stack[top] == '(') top--;
            continue;
        }

        if (c == '\'') {
            out[out_i++] = '\'';
            continue;
        }

        if (c == '+' || c == '*') {
            int p = highest(c);
            while (top >= 0 && stack[top] != '(' && highest(stack[top]) >= p) {
                out[out_i++] = stack[top--];
            }
            stack[++top] = c;
            continue;
        }
    }

    while (top >= 0) {
        out[out_i++] = stack[top--];
    }

    out[out_i] = '\0';
    free(stack);
    return out;
}

matrix_sf* evaluate_expr_sf(char name, char *expr, bst_sf *root) {
    char *postfix = infix2postfix_sf(expr);

    int len = 0;
    while (postfix[len] != '\0') len++;

    matrix_sf **stack = malloc(sizeof(matrix_sf*) * len);
    int top = -1;

    for (int i = 0; postfix[i] != '\0'; i++) {
        char c = postfix[i];

        if (c == ' ' || c == '\n') continue;

        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            matrix_sf *m = find_bst_sf(c, root);
            stack[++top] = m;
            continue;
        }

        if (c == '\'') {
            matrix_sf *A = stack[top--];

            matrix_sf *R = transpose_mat_sf(A);
            R->name = '#';

            stack[++top] = R;

            if (!((A->name >= 'A' && A->name <= 'Z') || (A->name >= 'a' && A->name <= 'z'))) free(A);

            continue;
        }

        if (c == '+') {
            matrix_sf *B = stack[top--];
            matrix_sf *A = stack[top--];

            matrix_sf *R = add_mats_sf(A, B);
            R->name = '#';

            stack[++top] = R;

            if (!((A->name >= 'A' && A->name <= 'Z') || (A->name >= 'a' && A->name <= 'z'))) free(A);
            if (!((B->name >= 'A' && B->name <= 'Z') || (B->name >= 'a' && B->name <= 'z'))) free(B);

            continue;
        }

        if (c == '*') {
            matrix_sf *B = stack[top--];
            matrix_sf *A = stack[top--];

            matrix_sf *R = mult_mats_sf(A, B);
            R->name = '#';

            stack[++top] = R;

            if (!((A->name >= 'A' && A->name <= 'Z') || (A->name >= 'a' && A->name <= 'z'))) free(A);
            if (!((B->name >= 'A' && B->name <= 'Z') || (B->name >= 'a' && B->name <= 'z'))) free(B);

            continue;
        }
    }

    matrix_sf *result = stack[top--];
    result->name = name;

    free(stack);

    return result;
}

matrix_sf *execute_script_sf(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    bst_sf *root = NULL;
    matrix_sf *result = NULL;
    char *line = NULL;
    size_t max = MAX_LINE_LEN;

    while (getline(&line, &max, file) != -1) {
        char *p = line;
        while (*p == ' ') p++;
        if (*p == '\n' || *p == '\0') continue;

        char name = *p;
        p++;
        while (*p == ' ') p++;
    }

    fclose(file);
    return result;
}

// This is a utility function used during testing. Feel free to adapt the code to implement some of
// the assignment. Feel equally free to ignore it.
matrix_sf *copy_matrix(unsigned int num_rows, unsigned int num_cols, int values[]) {
    matrix_sf *m = malloc(sizeof(matrix_sf)+num_rows*num_cols*sizeof(int));
    m->name = '?';
    m->num_rows = num_rows;
    m->num_cols = num_cols;
    memcpy(m->values, values, num_rows*num_cols*sizeof(int));
    return m;
}

// Don't touch this function. It's used by the testing framework.
// It's been left here in case it helps you debug and test your code.
void print_matrix_sf(matrix_sf *mat) {
    assert(mat != NULL);
    assert(mat->num_rows <= 1000);
    assert(mat->num_cols <= 1000);
    printf("%d %d ", mat->num_rows, mat->num_cols);
    for (unsigned int i = 0; i < mat->num_rows*mat->num_cols; i++) {
        printf("%d", mat->values[i]);
        if (i < mat->num_rows*mat->num_cols-1)
            printf(" ");
    }
    printf("\n");
}
