
int import();

const char* foo_exported() {
    return "present";
}

const char* bar_exported() {
    return "present";
}

const char* foo_imported() {
    import();
    return "present";
}

const char* bar_imported() {
    import();
    return "present";
}
