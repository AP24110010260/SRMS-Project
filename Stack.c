#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 100

typedef struct
{
    int items[MAX_SIZE];
    int top;
} Stack;

void initStack(Stack *s);
int isEmpty(Stack *s);
int isFull(Stack *s);
void push(Stack *s, int value);
int pop(Stack *s);
int peek(Stack *s);
void display(Stack *s);

int main()
{
    int choice, value;
    Stack s;
    initStack(&s);

    while (1)
    {
        printf("\n========================================\n");
        printf("     STACK SIMULATION PROJECT\n");
        printf("========================================\n");
        printf("1. Push\n");
        printf("2. Pop\n");
        printf("3. Peek\n");
        printf("4. Display Stack\n");
        printf("5. Exit\n");
        printf("========================================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("Enter value to push: ");
            scanf("%d", &value);
            push(&s, value);
            break;
        case 2:
            value = pop(&s);
            if (value != -1)
                printf("Popped: %d\n", value);
            break;
        case 3:
            value = peek(&s);
            if (value != -1)
                printf("Top element: %d\n", value);
            break;
        case 4:
            display(&s);
            break;
        case 5:
            printf("\nExiting program...\n");
            return 0;
        default:
            printf("\nInvalid choice!\n");
        }
    }
    return 0;
}

void initStack(Stack *s)
{
    s->top = -1;
}

int isEmpty(Stack *s)
{
    return s->top == -1;
}

int isFull(Stack *s)
{
    return s->top == MAX_SIZE - 1;
}

void push(Stack *s, int value)
{
    if (isFull(s))
    {
        printf("Stack Overflow!\n");
        return;
    }
    s->items[++(s->top)] = value;
    printf("Pushed %d to stack\n", value);
}

int pop(Stack *s)
{
    if (isEmpty(s))
    {
        printf("Stack Underflow!\n");
        return -1;
    }
    return s->items[(s->top)--];
}

int peek(Stack *s)
{
    if (isEmpty(s))
    {
        printf("Stack is empty!\n");
        return -1;
    }
    return s->items[s->top];
}

void display(Stack *s)
{
    if (isEmpty(s))
    {
        printf("\nStack is empty!\n");
        return;
    }
    printf("\n========================================\n");
    printf("Stack contents (top to bottom):\n");
    printf("========================================\n");
    for (int i = s->top; i >= 0; i--)
    {
        printf("  | %4d |\n", s->items[i]);
    }
    printf("  +------+\n");
    printf("Total elements: %d\n", s->top + 1);
}