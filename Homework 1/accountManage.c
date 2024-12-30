#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-------data.h---------------------------------------------------------------------------
typedef struct data_s
{
    char username[20];
    char password[20];
    int status;
    int flagBlock;
    int signIn;
} data_t;

// void showData(data_t data)
// {
//     printf("%s - %s - %d\n", data.username, data.password, data.status, data.flagBlock);
// }

data_t convert(char username[], char password[], int status, int flagBlock)
{
    data_t data;
    strcpy(data.username, username);
    strcpy(data.password, password);
    data.status = status;
    data.flagBlock = flagBlock;
    return data;
}

//----------llist.h-----------------------------------------------------------------
typedef struct node_s
{
    data_t data;
    struct node_s *next;
} node_t, *root_t;

// --------Create a new Node-------------------------------------------------------------
// return:
//  - SUCCESS: return a pointer to new Node
//  - FAIL   : return a NULL pointer
node_t *createNewNode(const data_t data)
{
    node_t *newNode = (node_t *)malloc(sizeof(node_t));
    if (newNode == NULL)
        return NULL;

    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

root_t llInit()
{
    return NULL;
}

// node_t *llSeek(root_t root, int index)
// {
//     node_t *p;
//     for (p = root; index > 0 && (p != NULL); index--)
//         p = p->next;
//     return p;
// }

//---------------Insert------------------------------------------

root_t llInsertHead(root_t root, const data_t data)
{
    node_t *pNewNode = createNewNode(data);
    pNewNode->next = root;
    return (root_t)pNewNode;
}

// root_t llInsertTail(root_t root, const data_t data)
// {
//     node_t *pNewNode = createNewNode(data);

//     if (root == NULL)
//         return (root_t)pNewNode;

//     node_t *p = NULL;
//     for (p = root; p->next != NULL; p = p->next)
//         ;
//     p->next = pNewNode;
//     return root;
// }

// root_t llInsertAfter(root_t root, node_t *pAElem, const data_t data)
// {
//     if (pAElem == NULL)
//         return root;

//     node_t *pNewNode = createNewNode(data);
//     pNewNode->next = pAElem->next;
//     pAElem->next = pNewNode;

//     return root;
// }

//--------------Delete------------------------------------------------

// root_t llDeleteHead(root_t root)
// {
//     if (root == NULL)
//         return NULL;

//     node_t *p = root->next;
//     free(root);
//     return (root_t)p;
// }

// root_t llDeleteTail(root_t root)
// {
//     if (root == NULL)
//         return NULL;
//     if (root->next == NULL)
//     {
//         free(root);
//         return NULL;
//     }

//     node_t *p;
//     // Find previous node of Tail
//     for (p = root; p->next->next != NULL; p = p->next)
//         ;

//     free(p->next);
//     p->next = NULL;
//     return (root_t)root;
// }

// root_t llDeleteAfter(root_t root, node_t *pA)
// {
//     if ((pA == NULL) || (pA->next == NULL))
//         return root;

//     node_t *pToDelElem = pA->next;
//     pA->next = pA->next->next;

//     free(pToDelElem);
//     return root;
// }

root_t llDeleteAll(root_t root)
{
    node_t *p = NULL;
    for (; root != NULL; root = p)
    {
        p = root->next;
        free(root);
    }
    return NULL;
}

// -------------Tools--------------------------------------------------------------
// int llLength(root_t root)
// {
//     int count;
//     for (count = 0; root != NULL; root = root->next)
//         count++;
//     return count;
// }

// root_t LLReverse(root_t root)
// {
//     node_t *cur, *prev;

//     for (cur = prev = NULL; root != NULL;)
//     {
//         cur = root;
//         root = root->next;
//         cur->next = prev;
//         prev = cur;
//     }
//     return root;
// }

// int isEmpty(root_t root) { return root == NULL; }

int main()
{

    FILE *fptr;
    fptr = fopen("user.txt", "r");

    if (fptr == NULL)
    {
        printf("Error: file not found to read!\n");
        return 0;
    }
    char username[20];
    char password[20];
    int status;
    int usersIndex = 0;
    data_t users[100] = {0};

    while (fscanf(fptr, "%[^:]:%[^:]:%d\n", username, password, &status) == 3)
    {
        users[usersIndex++] = convert(username, password, status, 0);
    }
    root_t lst = llInit();

    for (int i = 0; i < usersIndex; i++)
    {
        lst = llInsertHead(lst, users[i]);
    }
    // printf("%d\n", users[3]);
    // printf("%d\n", usersIndex);
    fclose(fptr);
    char usernameInput[20];
    char passwordInput[20];
    int statusInput;
    int num;
    int x = 1;
    int signIn = 0;
    do
    {
        printf("\nUSER MANAGEMENT PROGRAM\n----------------------------------- \n1. Register\n2. Sign in\n3. Search\n4. Sign out\nYour choice (1-4, other to quit):\n");
        scanf("%d", &num);
        getchar();
        switch (num)
        {
        case 1:
            printf("\nusername:");
            scanf("%s", &usernameInput);
            getchar();
            printf("password:");
            scanf("%s", &passwordInput);

            int flag = 0;
            for (node_t *p = lst; p != NULL; p = p->next)
            {
                if (strcmp(p->data.username, usernameInput) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 1)
            {
                printf("\nAccount existed \n");
            }
            else
            {
                data_t user = convert(usernameInput, passwordInput, 1, 0);
                lst = llInsertHead(lst, user);
                fptr = fopen("user.txt", "a");
                if (fptr == NULL)
                {
                    printf("Error: file not found towrite!\n");
                }
                else
                {
                    fprintf(fptr, "%s:%s:%d\n", user.username, user.password, user.status);
                    fclose(fptr);
                    printf("\nSuccessful registration \n");
                }
            }
            break;
        case 2:
            printf("\nusername:");
            scanf("%s", &usernameInput);
            getchar();
            printf("password:");
            scanf("%s", &passwordInput);
            int flagUsername = 0;
            int flagPassword = 0;
            int flagStatus;
            node_t *p1 = NULL;
            for (node_t *p = lst; p != NULL; p = p->next)
            {
                if (strcmp(p->data.username, usernameInput) == 0)
                {
                    flagUsername = 1;
                    flagStatus = p->data.status;
                    if (strcmp(p->data.password, passwordInput) == 0)
                    {
                        flagPassword = 1;
                    }
                    p1 = p;
                    break;
                }
            }
            if (flagUsername == 1)
            {
                if (flagStatus == 0)
                {
                    printf("\nAccount is blocked\n");
                }
                else
                {
                    if (flagPassword == 1)
                    {
                        signIn = 1;
                        printf("\nHello hust\n");
                    }
                    else
                    {
                        if (++p1->data.flagBlock > 2)
                        {
                            p1->data.status = 0;
                            fptr = fopen("user.txt", "w");
                            if (fptr == NULL)
                            {
                                printf("Error: file not found to write!\n");
                            }
                            else
                            {
                                for (node_t *p = lst; p != NULL; p = p->next)
                                {
                                    fprintf(fptr, "%s:%s:%d\n", p->data.username, p->data.password, p->data.status);
                                }

                                fclose(fptr);
                            }

                            printf("\nPassword is incorrect. Account is blocked\n");
                        }
                        else
                        {
                            printf("\nPassword is incorrect\n");
                        }
                    }
                }
            }
            else
            {
                printf("\nCannot find account\n");
            }
            break;
        case 3:
            if (signIn == 0)
            {
                printf("\nAccount is not sign in \n");
            }
            else
            {
                printf("\nusername:");
                scanf("%s", &usernameInput);
                int flagUsername = 0;
                int flagStatus;
                for (node_t *p = lst; p != NULL; p = p->next)
                {
                    if (strcmp(p->data.username, usernameInput) == 0)
                    {
                        flagUsername = 1;
                        flagStatus = p->data.status;
                        break;
                    }
                }
                if (flagUsername == 1)
                {
                    if (flagStatus == 1)
                    {
                        printf("\nAccount is active\n");
                    }
                    else
                    {
                        printf("\nAcount is block\n");
                    }
                }
                else
                {
                    printf("\nCannot find account\n");
                }
            }

            break;
        case 4:
            if (signIn == 0)
            {
                printf("\nAccount is not sign in \n");
            }
            else
            {
                printf("\nusername:");
                scanf("%s", &usernameInput);
                int flagUsername = 0;

                for (node_t *p = lst; p != NULL; p = p->next)
                {
                    if (strcmp(p->data.username, usernameInput) == 0)
                    {
                        flagUsername = 1;
                        break;
                    }
                }
                if (flagUsername == 1)
                {
                    signIn = 0;
                    printf("\nGoodbye hust \n");
                }
                else
                {
                    printf("\nCannot find account\n");
                }
            }
            break;
        default:
            printf("%d", 5);
            printf("exit!\n");
            llDeleteAll(lst);
            return 0;
        }
    } while (x == 1);
    return 0;
}
