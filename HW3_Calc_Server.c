/** 컴퓨터네트워크(SW) 4분반 소프트웨어학과 32202970 윤예진
* 소켓프로그래밍 과제 3 : 계산기 프로그램 (클라이언트)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BUF_SIZE 256

// 토큰의 종류
typedef enum TokType {
    Operand,
    Operator
} TokType;


//수식의 구성 요소(연산자, 피연산자 등)를 char* = element 타입으로 정의
typedef char* element;

// 노드 구조체
typedef struct Node {
    element data;               // 토큰
    TokType type;               // 타입 (연산자, 피연산자)
    struct Node* link;          // 다음 노드 가리키는 포인터
} Node;

// 스택 구조체
typedef struct Stack {
	Node* top;
} Stack;


// 큐 구조체
typedef struct Queue {
	Node* front, * rear;        // 큐의 front와 rear를 가리키는 포인터
    int size;                   // 큐의 노드의 개수, 초기값 = 0
} Queue;

Node* createNode(element item, TokType type);

Stack* createStack(void);
bool isStackEmpty(Stack* stack);
void push(Stack* stack, element item, TokType type);
void push_node(Stack* stack, Node* node);
Node* pop(Stack* stack);

Queue* createQueue(void);
bool isQEmpty(Queue* queue);
void enQueue(Queue* queue, element item, TokType type);
void enQueue_node(Queue* queue, Node* node);
Node* deQueue(Queue* queue);

bool isdigit_str(char* token);
Queue* fromInorderToPostorder(char* expr);
double calc_postorder(Queue* postorder);


// 서버: 클라이언트로부터 수식을 수신하여 중위->후위 표기로 변환. 결과 값을 계산하여 전송한다.
int main(void)
{
    int ss = -1;                        // server socket
    int sa = -1;                        // socket accepted
    char buf[BUF_SIZE];                 // 통신 버퍼
    char* tmp = NULL;
    Queue* postfix = NULL;              // 후위 표기식
    double result = 0;

    // 소켓 주소 설정 및 초기화
    struct sockaddr_in ssa;
    memset(&ssa, 0, sizeof(ssa));
    ssa.sin_family = AF_INET;                           // IPv4 주소 체계 설정
    ssa.sin_addr.s_addr = htonl(INADDR_ANY);
    ssa.sin_port = htons(12345);

    // 소켓 생성 후 바인딩
    ss = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(bind(ss, (struct sockaddr *) &ssa, sizeof(ssa)) < 0)
    {   // bind error
        printf("failed to bind socket\n");
        close(ss);  exit(1);
    }

    // 요청 대기
    listen(ss, 10);
    sa = accept(ss, 0, 0);

    // 클라이언트로부터 데이터 수신
    recv(sa, buf, BUF_SIZE, 0);

    // 개행 문자 제거
    tmp = strchr(buf, '\n');
    *tmp = '\0';

    // 중위 표기 -> 후위 표기 식 변환 후 계산
    postfix = fromInorderToPostorder(buf);
    result = calc_postorder(postfix);

    // 결과값 전송
    send(sa, (double *)&result, sizeof(double), 0);
    close(ss);
}


// 중위 표기식을 후위 표기식으로 변환하는 규칙
/* 1. 숫자는 그대로 큐에 삽입한다.
 * 2. 만약 스택이 비어있다면 연산자를 그냥 스택에 넣는다.
 * 3. (스택의 top에 있는 연산자의 우선순위 < 현재 연산자의 우선순위) 이면 현재 연산자를 그냥 스택에 넣는다.
 * 4. (스택의 top에 있는 연산자의 우선순위 >= 현재 연산자의 우선순위) 이면 2번 혹은 3번 상황이 될 때까지 pop 하여 큐에 삽입하고 연산자를 스택에 넣는다.
 * 5. 모든 수식을 다 사용했다면 스택이 빌 때까지 pop 하여 큐에 삽입한다.
 * 6. 우선순위는 (더하기=빼기) < (곱하기=나누기)이다.
 * 7. 여는 괄호는 스택에 그냥 추가한다.
 * 8. 여는 괄호 다음에 오는 연산자는 그냥 스택에 추가한다.
 * 9. 닫는 괄호는 여는 괄호가 나올 때까지 스택을 pop 하여 큐에 삽입한다. 다 출력하고 난 뒤 괄호들은 버린다.
 * 출처: https://todaycode.tistory.com/73
*/

// 중위 표기를 후위 표기식으로 변환한 것을 큐에 저장 후 리턴
Queue* fromInorderToPostorder(char* expr)
{
    Stack* tmpStk = createStack();           // 연산자를 임시 저장할 스택
    Queue* postfix = createQueue();                 // 후위 표기로 변환된 식을 저장하는 큐
    char* token = strtok(expr, " ");                // 공백으로 tokenize

    while((token!=NULL))
    {   
        // 피연산자 (연산자나 괄호가 아닌 경우)
        if (strcmp(token, "+")!=0 && strcmp(token, "-")!=0 &&
        strcmp(token, "/")!=0 && strcmp(token, "*")!=0 &&
        strcmp(token, "(")!=0 && strcmp(token, ")")!=0) {
            // 정수
            if(isdigit_str(token)) enQueue(postfix, token, Operand);
            // 실수
            else if(strchr(token, '.') > 0) enQueue(postfix, token, Operand);
            // 그 외
            else exit(1);
        }
        // 스택이 비어있다면 연산자를 그냥 스택에 넣는다.
        else if (isStackEmpty(tmpStk))
            push(tmpStk, token, Operator);

        // 닫는 괄호와 만나면 여는 괄호와 만날 때까지 스택을 pop 한다.
        else if (strcmp(token, ")") == 0) {
            while (isStackEmpty(tmpStk) == false && strcmp(tmpStk->top->data, "(")!=0)
                enQueue_node(postfix, pop(tmpStk));
            free(pop(tmpStk));       //여는 괄호 삭제
        }
        // 여는 괄호는 그냥 스택에 추가한다.
        else if (strcmp(token, "(") == 0)
            push(tmpStk, token, Operator);

        // 여는 괄호 뒤에 나오는 연산자는 그냥 스택에 추가한다.
        else if (strcmp(tmpStk->top->data, "(") == 0)
            push(tmpStk, token, Operator);

        // 규칙 3, 4번에 따라 연산자 처리

        // 스택 top: / or * 일 경우, 어떤 연산자(+ - / *)를 만나도 규칙 4번을 따른다.
        // 스택이 비거나 연산 우선순위가 더 높은 괄호를 만날 때까지 스택을 pop한 후 연산자를 스택에 추가한다.
        // 이때 여는 괄호는 pop하지 않는다.
        else if ((strcmp(tmpStk->top->data, "/") == 0) || (strcmp(tmpStk->top->data, "*") == 0)) {
            while (isStackEmpty(tmpStk) == false && strcmp(tmpStk->top->data, "(") != 0)
                enQueue_node(postfix, pop(tmpStk));
            push(tmpStk, token, Operator);
        }
        // 스택 top: + or - 일 경우, + -와 만났을 경우 규칙 4번을 따른다. / *와 만났을 경우 규칙 3번을 따른다. 
        else if ((strcmp(tmpStk->top->data, "+") == 0) || (strcmp(tmpStk->top->data, "-") == 0)) {
            // 규칙 4번을 따르는 경우. 여는 괄호는 pop하지 않는다.
            if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0) {
                while (isStackEmpty(tmpStk) == false && strcmp(tmpStk->top->data, "(") != 0)
                    enQueue_node(postfix, pop(tmpStk));
                push(tmpStk, token, Operator);
            }
            //규칙 3번을 따르는 경우. 만난 연산자를 그냥 스택에 추가한다.
            else if (strcmp(token, "/") == 0 || strcmp(token, "*") == 0)
                push(tmpStk, token, Operator);
        }
        token = strtok(NULL, " ");
    }
    // 규칙 5번에 따라 중위 표기식을 끝까지 조회했다면, 스택에 남아있는 것들을 pop한다.
    while (isStackEmpty(tmpStk) == false) enQueue_node(postfix, pop(tmpStk));

    free(tmpStk);
    return postfix;
}


// 후위 표기식을 계산 후 결과값을 리턴
// 피연산자는 스택에 저장.
// 연산자와 만날 경우 피연산자를 pop 하여 계산 후 연산 결과를 다시 스택에 push
double calc_postorder(Queue* postorder)
{
    Node* token = NULL;                 // 연산자 or 피연산자
    int QSize = postorder->size;        // 후위 표기식의 토큰 개수

    double* arrStack = calloc(QSize, sizeof(double));       // 임의로 사용할 스택. 배열로 구현.
    int stackTop = -1;                  //마지막에 넣은 원소의 위치 가리킴. 빈 스택일 시 -1

    char* oper = NULL;                  // 연산자
    double op1 = 0;                     // 첫번째 피연산자
    double op2 = 0;                     // 두번째 연산자
    double result = 0;                  // 연산 결과

    for(int i=0; i<QSize; i++)
    {
        token = deQueue(postorder);
        // 피연산자일 경우
        if (token->type != Operator) {
            stackTop++;
            arrStack[stackTop] = atof(token->data);     // 문자열 형태의 피연산자 --> 실수로 변환하여 저장
            free(token);
        }
        // 연산자일 경우
        else {
            oper = token->data; free(token);
            op2 = arrStack[stackTop]; arrStack[stackTop] = 0; stackTop--;
            op1 = arrStack[stackTop]; arrStack[stackTop] = 0; stackTop--;

            // op1 oper op2     ex) 10 - 4
            stackTop++;
            if(strcmp(oper, "+") == 0) arrStack[stackTop] = op1 + op2;
            else if(strcmp(oper, "-") == 0) arrStack[stackTop] = op1 - op2;
            else if(strcmp(oper, "*") == 0) arrStack[stackTop] = op1 * op2;
            else if(strcmp(oper, "/") == 0) arrStack[stackTop] = op1 / op2;
        }
    }

    // 연산 후 스택에 결과값만 존재하면 리턴
    if(stackTop == 0) {
        result = arrStack[stackTop]; free(arrStack);
        printf("Calc Result: %lf\ns", result);
        return result;
    }
    else {
        printf("postfix calc error: top %d\n", stackTop);
        exit(1);
    }
}


// 문자열이 숫자인지 검사
bool isdigit_str(char* token)
{
    int len = strlen(token);
    // 문자열의 각 문자에 대해 isdigit 으로 숫자 여부 검사
    for(int i=0; i<len; i++)
        if(!isdigit(token[i])) return false;
    return true;
}


// 노드를 생성하는 함수
// item, type 인자로 초기화, link의 초기값은 NULL.
Node* createNode(element item, TokType type)
{
    Node* tmp = (Node*)malloc(sizeof(Node));
    tmp->data = item;
    tmp->type = type;
    tmp->link = NULL;
    return tmp;
}

// 스택을 생성하는 함수
// top의 초기값은 NULL
Stack* createStack(void)
{
    Stack* tmp = (Stack*)malloc(sizeof(Stack));
    tmp->top = NULL;
    return tmp;
}


// 스택이 비어있는지 검사하는 함수
// 비어있다면 true, 아니라면 false 리턴
bool isStackEmpty(Stack* stack) {
    if (stack->top == NULL) return true;
    else return false;
}


// item, type을 멤버로 가지는 노드를 생성하여 스택에 push
// item은 밑의 top노드를 가리키고, top이 새로운 스택 노드를 가리키도록 한다.
void push(Stack* stack, element item, TokType type) {
    Node* temp = createNode(item, type);
    temp->link = stack->top;
    stack->top= temp;
}


// 이미 생성된 노드를 스택에 push
// item은 밑의 top노드를 가리키고, top이 새로운 스택 노드를 가리키도록 한다.
void push_node(Stack* stack, Node* node)  {
    node->link = stack->top;
    stack->top= node;
}


// top에 존재하는 노드를 제거하고 해당 노드 리턴. top이 제거된 노드 밑의 노드를 가리키도록 한다.
Node* pop(Stack* stack) {
    Node* tmp = stack->top;

    //스택이 비어있다면 에러메세지 출력
    if (isStackEmpty(stack)) {
        printf("\n\n Stack is empty !\n");
        return 0;
    }
    else {
        stack->top = tmp->link;
        return tmp;
    }
}


// 큐를 생성하는 함수
Queue* createQueue(void)
{
	Queue* tmp = (Queue*)malloc(sizeof(Queue));
	//malloc 할당에 실패 시
	if (tmp == NULL) {
		printf("새로운 큐 생성에 실패하였습니다.\n");
		return NULL;
    }
	tmp->front = NULL;
	tmp->rear = NULL;
    tmp->size = 0;

	return tmp;
}


// 큐의 공백 상태 여부 알려주는 함수. front가 NULL을 가리키면 공백.
// 공백이라면 true, 아니라면 false
bool isQEmpty(Queue* queue)
{
	if (queue->front == NULL) return true;
	else return false;
}


// 큐에 item, type을 원소를 가지는 노드를 생성하여 삽입하는 함수.
void enQueue(Queue* queue, element item, TokType type)
{
	Node* newNode = createNode(item, type);
	if (newNode == NULL) {
        printf("Failed to create new node\n");
        return;
    }

	//큐가 공백상태인 상태에서 새로 큐 노드를 삽입할 경우, rear와 front 모두 새로운 큐 노드를 가리키도록 한다.
	if (isQEmpty(queue)) {
		queue->rear = newNode;
		queue->front = newNode;
        queue->size++;
		return;
	}

	//큐가 공백이 아니라면, 마지막으로 삽입된 큐 노드와 rear가 새로운 큐 노드를 가리키도록 한다.
	queue->rear->link = newNode;
	queue->rear = newNode;
    queue->size++;
}


// 이미 생성된 노드를 큐에 삽입하는 함수
void enQueue_node(Queue* queue, Node* node)
{
	if (node == NULL) return;

	//큐가 공백상태인 상태에서 새로 큐 노드를 삽입할 경우, rear와 front 모두 새로운 큐 노드를 가리키도록 한다.
	if (isQEmpty(queue)) {
		queue->rear = node;
		queue->front = node;
        queue->size++;
		return;
	}

	//큐가 공백이 아니라면, 마지막으로 삽입된 큐 노드와 rear가 새로운 큐 노드를 가리키도록 한다.
	queue->rear->link = node;
	queue->rear = node;
    queue->size++;
}


// 큐의 노드를 삭제하고 해당 노드를 리턴하는 함수.
Node* deQueue(Queue* queue)
{
	// 큐가 공백 상태라면 에러 메세지 출력 후 종료
	if (isQEmpty(queue)) {
		printf("Queue is empty\n");
		return NULL;
	}
	Node* tmp = queue->front;
    queue->size--;

	// front가 삭제될 노드의 뒤의 노드를 가리키도록 한다. 삭제될 노드가 큐의 유일한 노드라면 front가 NULL을 가리키게 된다.
	queue->front = queue->front->link;
	// 삭제 후 노드가 존재하지 않는다면 rear가 NULL을 가리키도록 한다.
	if (isQEmpty(queue)) queue->rear = NULL;

	return tmp;
}