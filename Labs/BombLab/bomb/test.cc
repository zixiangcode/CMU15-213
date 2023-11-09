#include <ios>
#include <iostream>

// edx = 0xe
// esi = 0x0
// edi = x1
int fun4(int edx, int esi, int edi) {

    int eax = edx - esi; // 返回值
    eax = (eax + (eax >> 31)) >> 1;

    int ecx = eax + esi;
    if (edi > ecx) {
        esi = ecx + 1;
        return 2 * eax + 1 + fun4(edx, esi, edi);
    } else if (edi < ecx) {
        edx = ecx - 0x1;
        return 2 * fun4(edx, esi, edi);
    } else {
        return 0;
    }
}
// test eax -> eax = 0

struct Tree {
    int val;
    struct Tree *left;
    struct Tree *right;
};

int fun7(Tree *rdi, int esi) {
    if (!rdi)
        return -1;
    if (rdi->val == esi)
        return 0;
    else if (rdi->val < esi)
        return 2 * fun7(rdi->right, esi) + 1;
    else
        return 2 * fun7(rdi->left, esi);
}

int main() {

    for (int i = 0; i <= 0xe; i++) {
        if (fun4(0xe, 0x0, i) == 0) {
            std::cout << i << " \t";
        }
    }

    return 0;
}