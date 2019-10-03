/* Solution to comp10002 Assignment 2, 2019 semester 2.

   Authorship Declaration:

   (1) I certify that the program contained in this submission is completely
   my own individual work, except where explicitly noted by comments that
   provide details otherwise.  I understand that work that has been developed
   by another student, or by me in collaboration with other students,
   or by non-students as a result of request, solicitation, or payment,
   may not be submitted for assessment in this subject.  I understand that
   submitting for assessment work developed by or in collaboration with
   other students or non-students constitutes Academic Misconduct, and
   may be penalized by mark deductions, or by other penalties determined
   via the University of Melbourne Academic Honesty Policy, as described
   at https://academicintegrity.unimelb.edu.au.

   (2) I also certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor will
   I do so until after the marks are released. I understand that providing
   my work to other students, regardless of my intention or any undertakings
   made to me by that other student, is also Academic Misconduct.

   (3) I further understand that providing a copy of the assignment
   specification to any form of code authoring or assignment tutoring
   service, or drawing the attention of others to such services and code
   that may have been made available via such a service, may be regarded
   as Student General Misconduct (interfering with the teaching activities
   of the University and/or inciting others to commit Academic Misconduct).
   I understand that an allegation of Student General Misconduct may arise
   regardless of whether or not I personally make use of such solutions
   or sought benefit from such actions.

   Signed by: Christopher Chamberlain 1082551
   Dated:     3/10/19

*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define DEBUG 1

#define ROUTE_VALID 0
#define INITIAL_CELL_WRONG 1
#define GOAL_CELL_WRONG 2
#define ILLEGAL_MOVE 3
#define BLOCK_IN_ROUTE 4

typedef struct {
    int rows;
    int cols;
} dimensions_t;

typedef struct {
    int x;
    int y;
} point_t;

typedef struct {
    dimensions_t dim;
    int num_blocks;
    bool *inner;
} obstacle_grid_t;

struct node;

typedef struct node node_t;
typedef point_t list_item_t;

struct node {
    list_item_t coords;
    node_t *next;
    node_t *prev;
};

typedef struct {
    node_t *head;
    node_t *foot;
} list_t;

dimensions_t parse_dimensions();
void print_results(obstacle_grid_t *grid,
                   point_t start, point_t end, list_t *path);

bool parse_point(point_t *point);
bool same_point(point_t a, point_t b);

void obstacle_grid_alloc(obstacle_grid_t *grid, dimensions_t dim);
bool *obstacle_grid_get(obstacle_grid_t *grid, point_t point);
void obstacle_grid_free(obstacle_grid_t *grid);

list_t* ll_alloc();
bool ll_is_empty(list_t *list);
list_t* ll_insert_at_head(list_t *list, list_item_t data);
list_t* ll_insert_at_foot(list_t *list, list_item_t data);
void ll_free(list_t *list);

int
main(int argc, char *argv[]) {
    dimensions_t dim = parse_dimensions();

    obstacle_grid_t grid;
    obstacle_grid_alloc(&grid, dim);

    point_t start;
    parse_point(&start);
    scanf("\n");
    point_t end;
    parse_point(&end);
    scanf("\n");

    point_t point;
    while(parse_point(&point)) {
        bool *is_block = obstacle_grid_get(&grid, point);
        *is_block = true;
        grid.num_blocks++;
        scanf("\n");
    }

    scanf("$\n");

    list_t* path = ll_alloc();

    while(parse_point(&point)) {
        ll_insert_at_foot(path, point);
        scanf("->");
        scanf("\n");
    }

    print_results(&grid, start, end, path);

    ll_free(path);
    obstacle_grid_free(&grid);
	return 0;
}

int verify_route(obstacle_grid_t *grid,
                 point_t start, point_t end, list_t *path) {
    assert(!ll_is_empty(path));
    if(!same_point(path->head->coords, start)) {
        return INITIAL_CELL_WRONG;
    } else if(!same_point(path->foot->coords, end)) {
        return GOAL_CELL_WRONG;
    }

    node_t *step = path->head;
    bool block_in_route = false;
    while(step != NULL && step->next != NULL) {
        /* only allowed to move one cell in either direction each step */
        int difference = abs(step->coords.x - step->next->coords.x) +
                         abs(step->coords.y - step->next->coords.y);
        #if DEBUG
            fprintf(
                stderr, "difference: [%d,%d] and [%d, %d] = %d\n",
                step->coords.y, step->coords.x,
                step->next->coords.y, step->next->coords.x, difference
            );
        #endif

        if(*obstacle_grid_get(grid, step->coords)) {
            block_in_route = true;
        }
        if(difference > 1) {
            return ILLEGAL_MOVE;
        }

        step = step->next;
    }

    if(block_in_route) {
        return BLOCK_IN_ROUTE;
    }

    return ROUTE_VALID;
}

void print_results(obstacle_grid_t *grid,
                   point_t start, point_t end, list_t *path) {
    printf("==STAGE 0=======================================\n");
    printf("The grid has %d rows and %d columns.\n", grid->dim.rows, grid->dim.cols);
    printf("The grid has %d block(s).\n", grid->num_blocks);
    printf("The initial cell in the grid is [%d,%d].\n", start.y, start.x);
    printf("The goal cell in the grid is [%d,%d].\n", end.y, end.x);

    printf("The proposed route in the grid is:\n");
    node_t *step = path->head;
    int num_on_line = 0;
    while(step != NULL) {
        if(num_on_line >= 5) {
            num_on_line = 0;
            printf("\n");
        }
        printf("[%d,%d]", step->coords.y, step->coords.x);
        step = step->next;
        num_on_line++;
        if(step != NULL) {
            printf("->");
        } else {
            printf(".");
        }
    }
    printf("\n");

    int status = verify_route(grid, start, end, path);
    if(status == ROUTE_VALID) {
        printf("The route is valid!\n");
    } else if(status == INITIAL_CELL_WRONG) {
        printf("Initial cell in the route is wrong!\n");
    } else if(status == GOAL_CELL_WRONG) {
        printf("Goal cell in the route is wrong!\n");
    } else if(status == ILLEGAL_MOVE) {
        printf("There is an illegal move in this route!\n");
    } else if(status == BLOCK_IN_ROUTE) {
        printf("There is a block on this route!\n");
    }
}

dimensions_t parse_dimensions() {
    dimensions_t dim;
    int res = scanf("%dx%d\n", &dim.rows, &dim.cols);
    if(res != 2) {
        fprintf(stderr, "Failed to parse grid dimensions from stdin.\n");
        exit(EXIT_FAILURE);
    }
    return dim;
}

bool parse_point(point_t *point) {
    /* [r, c] where r is the row and c is the column */
    int res = scanf("[%d,%d]", &point->y, &point->x);
    if(res != 2) {
        return false;
    }
    return true;
}

bool same_point(point_t a, point_t b) {
    return a.x == b.x && a.y == b.y;
}

void obstacle_grid_alloc(obstacle_grid_t *grid, dimensions_t dim) {
    grid->dim = dim;
    grid->num_blocks = 0;
    grid->inner = (bool *) malloc(sizeof(int) * dim.rows * dim.cols);
    assert(grid->inner != NULL);
}

bool *obstacle_grid_get(obstacle_grid_t *grid, point_t point) {
    assert(grid->inner != NULL);
    assert(point.x < grid->dim.cols && point.y < grid->dim.rows);
    return &grid->inner[point.y * grid->dim.rows + point.x];
}

void obstacle_grid_free(obstacle_grid_t *grid) {
    free(grid->inner);
    grid->inner = NULL;
}


list_t* ll_alloc() {
    list_t *list = (list_t *) malloc(sizeof(*list));
    list->head = NULL;
    list->foot = NULL;
    return list;
}

bool ll_is_empty(list_t *list) {
    return list->head == NULL;
}

list_t* ll_insert_at_head(list_t *list, list_item_t coords) {
    node_t *node = (node_t *) malloc(sizeof(*node));
    node->coords = coords;
    node->prev = NULL;

    if(list->head == NULL) {
        node->next = NULL;
        list->head = node;
        list->foot = node;
    } else {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    }

    return list;
}

list_t* ll_insert_at_foot(list_t *list, list_item_t coords) {
    node_t *node = (node_t *) malloc(sizeof(*node));
    node->coords = coords;
    node->next = NULL;

    if(list->foot == NULL) {
        node->prev = NULL;
        list->head = node;
        list->foot = node;
    } else {
        list->foot->next = node;
        node->prev = list->foot;
        list->foot = node;
    }

    return list;
}

void ll_free(list_t *list) {
    node_t* node = list->head;
    while(node != NULL) {
        node_t *data = node;
        node = node->next;
        free(data);
    }

    free(list);
}