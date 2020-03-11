//  

#include <stdio.h>  
#include <stdlib.h>  
//#include <assert.h>  

//a pair of object.  
//There are two kind of objects, int and pair.The pair can obtain int and int, int and pair, or pair and pair  
typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType;

//tagged union  
//the definition of object.The object contains marked bit, ObjectType, the value and the pointer of next.  
//marked bit:used or not;  
//the pointer of next:the linked list  
//the value:because the object only contain one kind of ObjectType, we can use union to store.  
typedef struct sObject {
    ObjectType type;
    unsigned char marked;
    struct sObject* next;

    union {
        int value;
        struct {
            struct sObject* head;
            struct sObject* tail;
        };
    };
} Object;

//the definition of VM  
//it contain the numbers of object, the max numbers of object, stack, and the size of stack.  
#define STACK_SIZE 256  
typedef struct {
    int numObjects;
    int maxObjects;
    Object* firstObject;
    Object* stack[STACK_SIZE];
    int stacksize;
}VM;

#define ASSERT(condition, message) { \  
if (!(condition)) {
    \
        fprintf(stdout, "%s\n", message); \
} \
}
//create a new VM  
//initialization  
VM* newVM() {
    VM* vm = (VM*)malloc(sizeof(VM));
    vm->numObjects = 0;
    vm->maxObjects = 8;
    vm->firstObject = NULL;
    vm->stacksize = 0;
    return vm;
}

//push the object into stack  
void push(VM* vm, Object* value) {
    ASSERT(vm->stacksize < STACK_SIZE, "overflow");
    vm->stack[vm->stacksize++] = value;
}

//pop a object from the stack  
Object* pop(VM* vm) {
    ASSERT(vm->stacksize > 0, "underflow");
    Object* obj = vm->stack[--vm->stacksize];
    return obj;
}

//recursively mark the node  
void mark(Object* object) {
    int i, j;
    if (object == NULL) return;
    if (object->marked) return;

    object->marked = 1;

    if (object->type == OBJ_PAIR) {
        mark(object->tail);
        mark(object->head);
    }
}


//mark the all node  
void markAll(VM* vm) {
    int i;
    for (i = 0; i < vm->stacksize; ++i) {
        mark(vm->stack[i]);
    }
}

//free the unreached node.The node is marked 0.  
void sweep(VM* vm) {
    Object** obj = &(vm->firstObject);
    while (*obj) {
        if ((*obj)->marked == 0) {
            Object* unreached = *obj;
            *obj = unreached->next;
            free(unreached);
            --vm->numObjects;
        }
        else {
            (*obj)->marked = 0;
            obj = &((*obj)->next);
        }
    }
}

void gc(VM* vm) {
    int numObjects = vm->numObjects;
    markAll(vm);
    sweep(vm);

    //increase the maxObjects  
    vm->maxObjects = vm->numObjects * 2;

    fprintf(stdout, "Collected %d objects, %d remaining.\n", numObjects - vm->numObjects, vm->numObjects);
}

//new a object  
Object* newObject(VM* vm, ObjectType type) {
    if (vm->numObjects == vm->maxObjects)  gc(vm);

    Object* obj = (Object*)malloc(sizeof(Object));
    obj->type = type;
    obj->marked = 0;
    //insert into linked list form head  
    obj->next = vm->firstObject;
    vm->firstObject = obj;

    ++vm->numObjects;
    return obj;
}

//push the object of int into stack  
void pushInt(VM* vm, int value) {
    Object* obj = newObject(vm, OBJ_INT);
    obj->value = value;

    push(vm, obj);
}

Object* pushPair(VM* vm) {
    Object* obj = newObject(vm, OBJ_PAIR);
    obj->tail = pop(vm);
    obj->head = pop(vm);

    push(vm, obj);
    return obj; //take 1 day for this bug   
}

void test1() {
    fprintf(stdout, "Test1:Objects on stack are preserved.\n");
    VM* vm = newVM();
    pushInt(vm, 1);
    pushInt(vm, 2);

    gc(vm);
}

void test2() {
    fprintf(stdout, "Test2: Unreached objects are collected.\n");
    VM* vm = newVM();
    pushInt(vm, 1);
    pushInt(vm, 2);
    pop(vm);
    pop(vm);

    gc(vm);
}

void test3() {
    fprintf(stdout, "Test3: Reach nested objects.\n");
    VM* vm = newVM();
    pushInt(vm, 1);
    pushInt(vm, 2);
    Object* a = pushPair(vm);
    pushInt(vm, 3);
    pushInt(vm, 4);
    Object* b = pushPair(vm);
    Object* c = pushPair(vm);

    gc(vm);
}

void test4() {
    fprintf(stdout, "Test4: Handle cycle.\n");
    VM* vm = newVM();
    pushInt(vm, 1);
    pushInt(vm, 2);
    Object* a = pushPair(vm);
    pushInt(vm, 3);
    pushInt(vm, 4);
    Object* b = pushPair(vm);

    a->tail = b;
    b->tail = a;

    gc(vm);
}

void perfTest() {
    fprintf(stdout, "Performance Test.\n");
    VM* vm = newVM();
    int i, j, k;
    for (i = 0; i < 1000; ++i) {
        for (j = 0; j < 20; ++j) {
            pushInt(vm, j);
        }
        for (k = 0; k < 20; ++k) {
            pop(vm);
        }
    }
}

int main(void) {
      test1();  
    //  test2();  
    //  test3();  
    //  test4();  

    //perfTest();

    return 0;
}