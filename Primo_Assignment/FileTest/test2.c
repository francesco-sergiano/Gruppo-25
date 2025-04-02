// test2.c
int foo(int a, int b) {
    // Operazioni dove si trova "+ 0"
    int x = a + 0;
    int y = 0 + b;  // caso opposto
    return x + y;
}

int main() {
    return foo(10,20);
}
