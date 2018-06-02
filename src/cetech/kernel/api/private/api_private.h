#ifndef CETECH__API_H
#define CETECH__API_H



struct ct_alloc;
struct ct_api_a0;

void api_init(struct ct_alloc *allocator);

void api_shutdown();

struct ct_api_a0 *api_v0();


#endif //CETECH__API_H
