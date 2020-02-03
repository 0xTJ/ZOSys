#ifndef INCLUDE_MODULE_H
#define INCLUDE_MODULE_H

struct module {
    int (*init)(void);
    void (*exit)(void);
};

int module_init(struct module *module);
#define module_init(module) ((module)->init())
void module_exit(struct module *module);
#define module_exit(module) ((module)->exit())

#endif
