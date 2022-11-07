#ifndef QUEUE_H_
#define QUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FULL 100 /*Gia tri nay thuoc vao gioi han cua QUEUE ma ban muon cai dat*/
#ifndef STRUCT_DT
#define STRUCT_DT
typedef struct DT
{
    int point;
} DataType;
typedef DataType ElementType;
// const ElementType NotFound = {"Not Found", "Not Found", "Not Found"};
#endif
#ifndef STRUCT_LKD
#define STRUCT_LKD
typedef struct LKD
{
    ElementType data;
    struct LKD *next;
} LinkList;
#endif
typedef struct QUEUE
{
    LinkList *Front;
    LinkList *Rear;
} QUEUE;
/// @brief
/// @param
/// @return
QUEUE *MakeNullQueue(void); /*Khoi tao mot QUEUE rong*/
/// @brief Tra ve so phan tu trong QUEUE
/// @param  QUEUE* Q
/// @return
int sizeQ(QUEUE *);
/// @brief Kiem tra QUEUE co rong khong
/// @param  QUEUE* Q
/// @return boolean
int isEmptyQ(QUEUE *);
/// @brief Kiem tra QUEUE co day khong
/// @param  QUEUE* Q
/// @return boolean
int isFullQ(QUEUE *);
/// @brief Tra ve con tro cua phan tu FRONT
/// @param  QUEUE* Q
/// @return LinkList *Front
LinkList *front(QUEUE *);
/// @brief Them phan tu vao phia sau QUEUE
/// @param  QUEUE* Q, ElementType x
/// @return void
void EnQueue(QUEUE *, ElementType);
/// @brief Xoa phan tu o phia truoc QUEUE va tra ve data cua phan tu do
/// @param  QUEUE* Q
/// @return ElementType
ElementType DeQueue(QUEUE *);

void PrintQ(QUEUE *); /*In ra danh sach cua cac phan tu trong QUEUE*/

QUEUE *MakeNullQueue(void)
{
    QUEUE *New;
    New = (QUEUE *)malloc(sizeof(QUEUE));
    LinkList *root;
    root = (LinkList *)malloc(sizeof(LinkList));
    root->next = NULL;
    // root->data = NotFound;
    New->Rear = root;
    New->Front = root;
    return New;
}
int sizeQ(QUEUE *Q)
{
    int i = 0;
    LinkList *root;
    root = Q->Front;
    while (root != Q->Rear)
    {
        root = root->next;
        i++;
    }
    return i;
}
int isEmptyQ(QUEUE *Q)
{
    return (Q->Front == Q->Rear);
}
int isFullQ(QUEUE *Q)
{
    return (sizeQ(Q) == FULL);
}
LinkList *front(QUEUE *Q)
{
    if (!isEmptyQ(Q))
    {
        return Q->Front;
    }
    else
    {
        return NULL;
    }
}
void EnQueue(QUEUE *Q, ElementType x)
{
    if (!isFullQ(Q))
    {
        Q->Rear->next = (LinkList *)malloc(sizeof(LinkList));
        Q->Rear = Q->Rear->next;
        Q->Rear->data = x;
        Q->Rear->next = NULL;
    }
    else
    {
        printf("Full Queue!\n");
    }
}
ElementType DeQueue(QUEUE *Q)
{
    if (!isEmptyQ(Q))
    {
        ElementType x;
        LinkList *Head;
        Head = Q->Front;
        Q->Front = Q->Front->next;
        x = Q->Front->data;
        free(Head);
        return x;
    }
    else
    {
        printf("Empty Queue!\n");
        free(Q->Front);
        free(Q);
        exit(0);
    }
}
void PrintQ(QUEUE *Q) /*Ham nay tuy thuoc vao cau truc du lieu cua data*/
{
    /* code */
}
#endif