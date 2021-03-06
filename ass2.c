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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define VEC_INITIAL_CAPACITY 2
#define DSEPARATOR "================================================\n"
#define STAGE_ZERO "==STAGE 0=======================================\n"
#define STAGE_ONE "==STAGE 1=======================================\n"
#define STAGE_TWO "==STAGE 2=======================================\n"
#define SSEPARATOR "------------------------------------------------\n"

#define ROUTE_VALID 5
#define INITIAL_CELL_WRONG 1
#define GOAL_CELL_WRONG 2
#define ILLEGAL_MOVE 3
#define BLOCK_IN_ROUTE 4

#define EMPTY_CELL -1
#define BLOCK_CELL -2
#define INITIAL_CELL -3
#define GOAL_CELL -4
#define ROUTE_CELL -5
#define SEARCH_CELL_MIN 0

typedef struct {
    int rows;
    int cols;
} dimensions_t;

typedef struct {
    int x;
    int y;
} point_t;

/* path-finding grid */

typedef int cell_t;
typedef struct {
    dimensions_t dim;
    cell_t *inner;
} grid_t;

void grid_init(grid_t *grid, dimensions_t dim);
void grid_clear(grid_t *grid);
cell_t grid_get(grid_t *grid, point_t point);
void grid_set(grid_t *grid, point_t point, cell_t cell);
void grid_copy(grid_t *src, grid_t *dest);
void grid_free(grid_t *grid);

/* a doubly-linked list */

struct node;

typedef struct node node_t;
typedef point_t list_item_t;

/* I chose to avoid the use of void* data types here for two reasons:
   (1) I dislike how void* strips all type information, so you have to hope
       the data is interpreted as the correct type. (compiler won't complain)
   (2) I want to avoid as much pointer indirection as possible, and using
       a void* means that the list_item_t must be allocated separately to the
       node_t. */
struct node {
    list_item_t coords;
    node_t *next;
    node_t *prev;
};

typedef struct {
    node_t *head;
    node_t *foot;
} list_t;

void ll_init(list_t *list);
bool ll_is_empty(list_t *list);
node_t *ll_insert_before(list_t *list, node_t *before, list_item_t coords);
node_t *ll_insert_after(list_t *list, node_t *after, list_item_t coords);
void ll_remove(list_t *list, node_t *node);
void ll_free(list_t *list);

typedef struct {
    point_t coords;
    int distance;
} vec_item_t;

/*  A growable ring buffer,
    inspired by VecDeque from the Rust stdlib:
    https://doc.rust-lang.org/std/collections/struct.VecDeque.html */
typedef struct {
    vec_item_t *storage;
    /* where data should be written */
    size_t head;
    /* first data which can be read */
    size_t tail;
    size_t capacity;
} vec_deque_t;

void vec_init(vec_deque_t *vec);
void vec_push_back(vec_deque_t *vec, vec_item_t item);
bool vec_pop_front(vec_deque_t *vec, vec_item_t *item);
void vec_free(vec_deque_t *vec);

typedef struct {
    point_t start;
    point_t end;
    list_t steps;
} path_t;

void print_results(grid_t *grid, path_t *path, int status, bool initial);
void print_list(list_t *list);
void print_grid(grid_t *grid);
void print_path_on_grid(grid_t *grid, grid_t *scratch_grid, path_t *path);
void print_point(point_t point);

int verify_route(grid_t *grid, path_t *path);
void iter_repair_route(grid_t *grid, grid_t *scratch_grid, path_t *path,
                       int *status, bool iter);
void find_repair_start(grid_t *grid, path_t *path, node_t **repair_start,
                       node_t **tail_path_start);
bool repair_route(grid_t *grid, path_t *path);
bool try_search_step(grid_t *grid, point_t point, int distance,
                     vec_deque_t *repair, point_t *repair_end);
bool try_backtrack_step(grid_t *grid, point_t *point, int *min_distance,
                        point_t try_point);
void draw_path(grid_t *grid, node_t *start_path);
bool parse_point(point_t *point);
void parse_initial(grid_t *grid, path_t *path);
dimensions_t parse_dimensions();
bool same_point(point_t a, point_t b);

int main(int argc, char *argv[]) {
    grid_t grid;
    path_t path;

    parse_initial(&grid, &path);

    int status = verify_route(&grid, &path);

    /* stage 0 results */
    printf(STAGE_ZERO);
    print_results(&grid, &path, status, true);

    grid_t scratch_grid;
    grid_init(&scratch_grid, grid.dim);

    /* stage 1 results */
    printf(STAGE_ONE);
    iter_repair_route(&grid, &scratch_grid, &path, &status, false);

    bool first = true;
    /* consume any whitespace */
    scanf(" ");
    /* proceed to stage 2 */
    while ((status == ROUTE_VALID || status == BLOCK_IN_ROUTE) &&
           getchar() == '$') {
        if (first) {
            printf(STAGE_TWO);
            first = false;
        } else {
            printf(DSEPARATOR);
        }

        scanf(" ");

        grid_clear(&grid);
        point_t point;
        while (parse_point(&point)) {
            grid_set(&grid, point, BLOCK_CELL);
            scanf(" ");
        }
        status = verify_route(&grid, &path);
        iter_repair_route(&grid, &scratch_grid, &path, &status, true);
        scanf(" ");
    }

    printf(DSEPARATOR);

    /* cleanup */
    ll_free(&path.steps);
    grid_free(&grid);
    grid_free(&scratch_grid);
    return 0;
}

/* parses stage one input */
void parse_initial(grid_t *grid, path_t *path) {
    dimensions_t dim = parse_dimensions();
    grid_init(grid, dim);

    point_t start;
    int res = parse_point(&start);
    assert(res);
    scanf("\n");
    point_t end;
    res = parse_point(&end);
    assert(res);
    scanf("\n");

    point_t point;
    while (parse_point(&point)) {
        grid_set(grid, point, BLOCK_CELL);
        scanf(" ");
    }

    scanf("$ ");

    path->start = start;
    path->end = end;
    ll_init(&path->steps);

    while (parse_point(&point)) {
        ll_insert_after(&path->steps, path->steps.foot, point);
        scanf("-> "); /* allows for whitespace e.g.: newlines after an arrow */
    }
}

void iter_repair_route(grid_t *grid, grid_t *scratch_grid, path_t *path,
                       int *status, bool iter) {
    print_path_on_grid(grid, scratch_grid, path);

    /* only iteratively repair if there were blocks in the way to start with */
    while (*status == BLOCK_IN_ROUTE) {
        grid_copy(grid, scratch_grid);
        bool repaired = repair_route(scratch_grid, path);

        if (!repaired) {
            /* if the grid was broken and could not be repaired */
            printf(SSEPARATOR);
            print_path_on_grid(grid, scratch_grid, path);
            printf(SSEPARATOR);
            printf("The route cannot be repaired!\n");
            return;
        }

        *status = verify_route(grid, path);
        if (*status == ROUTE_VALID || !iter) {
            /* if the route was broken and successfully repaired,
               or one repair was successful during stage one */
            printf(SSEPARATOR);
            print_path_on_grid(grid, scratch_grid, path);
            printf(SSEPARATOR);
            print_results(grid, path, *status, false);
            return;
        }
    }
}

/* returns the status of the given `path` on the `grid`. */
int verify_route(grid_t *grid, path_t *path) {
    assert(!ll_is_empty(&path->steps));
    if (!same_point(path->steps.head->coords, path->start)) {
        return INITIAL_CELL_WRONG;
    } else if (!same_point(path->steps.foot->coords, path->end)) {
        return GOAL_CELL_WRONG;
    }

    node_t *step = path->steps.head;
    bool block_in_route = false;
    while (step != NULL && step->next != NULL) {
        /* only allowed to move one cell in either direction each step */
        int difference = abs(step->coords.x - step->next->coords.x) +
                         abs(step->coords.y - step->next->coords.y);
        /* but first check we aren't accessing out-of-bounds */
        if (step->coords.x < 0 || step->coords.x >= grid->dim.cols ||
            step->coords.y < 0 || step->coords.y >= grid->dim.rows) {
            return ILLEGAL_MOVE;
        }
        if (grid_get(grid, step->coords) == BLOCK_CELL) {
            block_in_route = true;
        }
        if (difference > 1) {
            return ILLEGAL_MOVE;
        }

        step = step->next;
    }

    if (block_in_route) {
        return BLOCK_IN_ROUTE;
    }

    return ROUTE_VALID;
}

void find_repair_start(grid_t *grid, path_t *path, node_t **repair_start,
                       node_t **tail_path_start) {
    assert(!ll_is_empty(&path->steps));
    node_t *step = path->steps.head;

    /* get to the start of the repair */
    while (step->next != NULL &&
           grid_get(grid, step->next->coords) != BLOCK_CELL) {
        step = step->next;
    }

    *repair_start = step;
#if DEBUG
    fprintf(stderr, "Starting repair at: [%d, %d]", repair_start->coords.y,
            repair_start->coords.x);
#endif

    /* skip the starting cell */
    if (step->next != NULL) {
        step = step->next;
    }
    /* skip the problem cells */
    while (step->next != NULL && grid_get(grid, step->coords) == BLOCK_CELL) {
        step = step->next;
    }
    *tail_path_start = step;
}

/* repairs a route.
   `grid` should be a scratch grid which only contains block cells.
   if the function returns false, then `path` will not be modified */
bool repair_route(grid_t *grid, path_t *path) {
    /* `repair_start` is the final route cell before the obstacle.
       `tail_path_start` is the first route cell after the obstacle */
    node_t *repair_start, *tail_path_start;
    find_repair_start(grid, path, &repair_start, &tail_path_start);

    /* our repair should find path segments *after* the broken region
       so we draw only those path segments onto the grid */
    draw_path(grid, tail_path_start);
    /* override the starting cell to be `0` */
    grid_set(grid, repair_start->coords, SEARCH_CELL_MIN);

#if DEBUG
    fprintf(stderr, ", ending somewhere after: [%d, %d]\n",
            tail_path_start->coords.y, tail_path_start->coords.x);
#endif

    vec_deque_t repair;
    vec_init(&repair);
    vec_item_t search_start = {.coords = repair_start->coords, .distance = 0};
    vec_push_back(&repair, search_start);

    /* begin searching for the 'tail path' that we want to link up to */
    point_t repair_end;
    bool tail_found = false;
    int dist = 0;
    vec_item_t current;
    while (!tail_found && vec_pop_front(&repair, &current)) {
        point_t p = current.coords;
        point_t above = {.x = p.x, .y = p.y - 1};
        point_t below = {.x = p.x, .y = p.y + 1};
        point_t left = {.x = p.x - 1, .y = p.y};
        point_t right = {.x = p.x + 1, .y = p.y};
        dist = current.distance + 1;
        /* search with priority above, below, left, right */
        tail_found =
            (above.y >= 0 &&
             try_search_step(grid, above, dist, &repair, &repair_end)) ||
            (below.y < grid->dim.rows &&
             try_search_step(grid, below, dist, &repair, &repair_end)) ||
            (left.x >= 0 &&
             try_search_step(grid, left, dist, &repair, &repair_end)) ||
            (right.x < grid->dim.cols &&
             try_search_step(grid, right, dist, &repair, &repair_end));
    }

    vec_free(&repair);

    if (!tail_found) {
        return false;
    }

#if DEBUG
    fprintf(stderr, "Found route, ended at: [%d, %d]\n", repair_end.y,
            repair_end.x);
    print_grid(grid);
#endif

    /* don't remove the starting cell */
    if (repair_start->next != NULL) {
        tail_path_start = repair_start->next;
    }
    /* remove parts of the old path, till we get to the end of the repair */
    while (tail_path_start->next != NULL &&
           !same_point(tail_path_start->coords, repair_end)) {
        tail_path_start = tail_path_start->next;
        ll_remove(&path->steps, tail_path_start->prev);
    }

    point_t tail_point = repair_end;

    /* gradually add cells until the start of the tail path
       joins up with the start of the repair */
    while (!same_point(tail_point, repair_start->coords)) {
        if (!same_point(tail_point, repair_end)) {
            tail_path_start =
                ll_insert_before(&path->steps, tail_path_start, tail_point);
        }

        point_t above = {.x = tail_point.x, .y = tail_point.y - 1};
        point_t below = {.x = tail_point.x, .y = tail_point.y + 1};
        point_t left = {.x = tail_point.x - 1, .y = tail_point.y};
        point_t right = {.x = tail_point.x + 1, .y = tail_point.y};

        /* backtrack with priority above, below, left, right */
        bool backtracked =
            (above.y >= 0 &&
             try_backtrack_step(grid, &tail_point, &dist, above)) ||
            (below.y < grid->dim.rows &&
             try_backtrack_step(grid, &tail_point, &dist, below)) ||
            (left.x >= 0 &&
             try_backtrack_step(grid, &tail_point, &dist, left)) ||
            (right.x < grid->dim.cols &&
             try_backtrack_step(grid, &tail_point, &dist, right));

        /* sanity check that we're getting closer to start.
           if not, we terminate the program instead of entering an
           infinite loop */
        if (!backtracked) {
            fprintf(stderr, "Backtracking operation will not terminate.\n");
            exit(EXIT_FAILURE);
        }
    }

    return true;
}

/* tries to extend the search region to a cell at `point` */
bool try_search_step(grid_t *grid, point_t point, int distance,
                     vec_deque_t *repair, point_t *repair_end) {
    int cell = grid_get(grid, point);
    if (cell == EMPTY_CELL) {
        vec_item_t next = {.distance = distance, .coords = point};
        grid_set(grid, point, SEARCH_CELL_MIN + next.distance);
        vec_push_back(repair, next);
    } else if (cell == ROUTE_CELL) {
        *repair_end = point;
        return true;
    }
    return false;
}

/* tries to backtrack from to the cell at `try_point` */
bool try_backtrack_step(grid_t *grid, point_t *point, int *min_distance,
                        point_t try_point) {
    int cell = grid_get(grid, try_point);
    if (cell >= SEARCH_CELL_MIN) {
        int distance = cell - SEARCH_CELL_MIN;
        if (distance <= *min_distance) {
            *point = try_point;
            *min_distance = distance;
            return true;
        }
    }
    return false;
}

void draw_path(grid_t *grid, node_t *start_path) {
    /* draw the path into the 2D array*/
    node_t *step = start_path;
    /* if a visited cell is also the initial cell, goal cell, or contains a
       block, then `I`, `G`, or `#`, respectively, should be printed, not `*` */
    while (step != NULL) {
        cell_t cell = grid_get(grid, step->coords);
        if (cell != BLOCK_CELL) {
            grid_set(grid, step->coords, ROUTE_CELL);
        }
        step = step->next;
    }
}

void print_path_on_grid(grid_t *grid, grid_t *scratch_grid, path_t *path) {
    grid_copy(grid, scratch_grid);
    draw_path(scratch_grid, path->steps.head);
    grid_set(scratch_grid, path->start, INITIAL_CELL);
    grid_set(scratch_grid, path->end, GOAL_CELL);
    print_grid(scratch_grid);
}

void print_grid(grid_t *grid) {
    int i, j;
    for (j = -1; j < grid->dim.rows; j++) {
        for (i = -1; i < grid->dim.cols; i++) {
            if (i == -1 && j == -1) {
                printf(" ");
            } else if (i == -1) {
                printf("%d", j % 10);
            } else if (j == -1) {
                printf("%d", i % 10);
            } else {
                point_t coords = {.x = i, .y = j};
                cell_t cell = grid_get(grid, coords);
                if (cell == EMPTY_CELL) {
                    printf(" ");
                } else if (cell == BLOCK_CELL) {
                    printf("#");
                } else if (cell == ROUTE_CELL) {
                    printf("*");
                } else if (cell == INITIAL_CELL) {
                    printf("I");
                } else if (cell == GOAL_CELL) {
                    printf("G");
                } else if (cell >= SEARCH_CELL_MIN) {
                    printf("%d", (cell - SEARCH_CELL_MIN) % 10);
                }
            }
        }
        printf("\n");
    }
}

void print_point(point_t point) { printf("[%d,%d]", point.y, point.x); }

void print_results(grid_t *grid, path_t *path, int status, bool initial) {
    if (initial) {
        /* calculate the number of blocks on-demand */
        int num_blocks = 0;
        int i, j;
        for (i = 0; i < grid->dim.cols; i++) {
            for (j = 0; j < grid->dim.rows; j++) {
                point_t point = {.x = i, .y = j};
                if (grid_get(grid, point) == BLOCK_CELL) {
                    num_blocks++;
                }
            }
        }
        printf("The grid has %d rows and %d columns.\n", grid->dim.rows,
               grid->dim.cols);
        printf("The grid has %d block(s).\n", num_blocks);
        printf("The initial cell in the grid is ");
        print_point(path->start);
        printf(".\nThe goal cell in the grid is ");
        print_point(path->end);
        printf(".\nThe proposed route in the grid is:\n");
    }
    print_list(&path->steps);
    if (status == ROUTE_VALID) {
        printf("The route is valid!\n");
    } else if (status == INITIAL_CELL_WRONG) {
        printf("Initial cell in the route is wrong!\n");
    } else if (status == GOAL_CELL_WRONG) {
        printf("Goal cell in the route is wrong!\n");
    } else if (status == ILLEGAL_MOVE) {
        printf("There is an illegal move in this route!\n");
    } else if (status == BLOCK_IN_ROUTE) {
        printf("There is a block on this route!\n");
    }
}

void print_list(list_t *list) {
    node_t *step = list->head;
    int num_on_line = 0;
    while (step != NULL) {
        if (num_on_line >= 5) {
            num_on_line = 0;
            printf("\n");
        }
        print_point(step->coords);
        step = step->next;
        num_on_line++;
        if (step != NULL) {
            printf("->");
        } else {
            printf(".");
        }
    }
    printf("\n");
}

dimensions_t parse_dimensions() {
    dimensions_t dim;
    int res = scanf("%dx%d\n", &dim.rows, &dim.cols);
    if (res != 2) {
        fprintf(stderr,
                "Fatal error: failed to parse grid dimensions from stdin.\n");
        exit(EXIT_FAILURE);
    }
    return dim;
}

bool parse_point(point_t *point) {
    /* [r, c] where r is the row and c is the column */
    int res = scanf("[%d,%d]", &point->y, &point->x);
    if (res != 2) {
        return false;
    }
    return true;
}

bool same_point(point_t a, point_t b) { return a.x == b.x && a.y == b.y; }

/* Initialises an empty grid_t in-place.
   I initialise all my data structures in-place for this assignment because:
   (1) the fixed-size 'container' types (grid_t, list_t, vec_deque_t) can be
       stack-allocated, avoiding unneeded mallocs, whilst the dynamic
       internals (the node_t's, cell_t array and vec_item_t array) are malloced
       and freed as usual. This makes it more similar to e.g.: the C++ STL
   (2) I'm not exposing this code as a standalone library, so the fact you need
       to know the sizes of the types in order to allocate on the stack isn't
       a problem. */
void grid_init(grid_t *grid, dimensions_t dim) {
    grid->dim = dim;
    size_t bytes = sizeof(cell_t) * dim.rows * dim.cols;
    grid->inner = (cell_t *)malloc(bytes);
    assert(grid->inner != NULL);
    grid_clear(grid);
}

void grid_clear(grid_t *grid) {
    /* 'zeroes' the memory with empty cells */
    size_t bytes = sizeof(cell_t) * grid->dim.rows * grid->dim.cols;
    memset(grid->inner, EMPTY_CELL, bytes);
}

cell_t *grid_get_ptr(grid_t *grid, point_t point) {
    assert(grid->inner != NULL);
    assert(point.x >= 0 && point.x < grid->dim.cols && point.y >= 0 &&
           point.y < grid->dim.rows);
    return &grid->inner[point.y * grid->dim.cols + point.x];
}

cell_t grid_get(grid_t *grid, point_t point) {
    if(point.x >= 0 && point.x < grid->dim.cols && point.y >= 0 &&
           point.y < grid->dim.rows) {
        return *grid_get_ptr(grid, point);
    } else {
        return EMPTY_CELL;
    }
}

/* all out of bounds coordinates are ignored */
void grid_set(grid_t *grid, point_t point, cell_t cell) {
    if(point.x >= 0 && point.x < grid->dim.cols && point.y >= 0 &&
           point.y < grid->dim.rows) {
        *grid_get_ptr(grid, point) = cell;
    }
}

/* copies the contents of `src` into `dest`. note: both grids must already be
 * initialised to identical dimensions. */
void grid_copy(grid_t *src, grid_t *dest) {
    assert(src->dim.cols == dest->dim.cols && src->dim.rows == dest->dim.rows);
    size_t bytes = sizeof(cell_t) * src->dim.rows * src->dim.cols;
    memcpy(dest->inner, src->inner, bytes);
}

void grid_free(grid_t *grid) {
    free(grid->inner);
    grid->inner = NULL;
}

void ll_init(list_t *list) {
    list->head = NULL;
    list->foot = NULL;
}

bool ll_is_empty(list_t *list) { return list->head == NULL; }

/* Here I return the node_t* which is inserted - this could be seen as
   'exposing implementation details', however once again this code isn't
   written as a library, and having access to each node_t directly removes the
   need to traverse the list at all to search for items. */
node_t *ll_insert_before(list_t *list, node_t *before, list_item_t coords) {
    node_t *node = (node_t *)malloc(sizeof(*node));
    assert(node != NULL);
    node->coords = coords;
    node->next = before;

    if (before == NULL) {
        /* the list must be empty */
        assert(ll_is_empty(list));
        list->head = node;
        list->foot = node;
        node->prev = NULL;
    } else {
        node->prev = before->prev;
        if (list->head == before) {
            list->head = node;
        } else {
            assert(node->prev != NULL);
            node->prev->next = node;
        }
        before->prev = node;
    }

    return node;
}

node_t *ll_insert_after(list_t *list, node_t *after, list_item_t coords) {
    node_t *node = (node_t *)malloc(sizeof(*node));
    assert(node != NULL);
    node->coords = coords;
    node->prev = after;

    if (after == NULL) {
        /* the list must be empty */
        assert(ll_is_empty(list));
        list->head = node;
        list->foot = node;
        node->next = NULL;
    } else {
        node->next = after->next;
        if (list->foot == after) {
            list->foot = node;
        } else {
            assert(node->next != NULL);
            node->next->prev = node;
        }
        after->next = node;
    }

    return node;
}

void ll_remove(list_t *list, node_t *node) {
    if (list->head == node && list->foot == node) {
        list->head = NULL;
        list->foot = NULL;
    } else if (list->head == node) {
        node->next->prev = NULL;
        list->head = node->next;
    } else if (list->foot == node) {
        node->prev->next = NULL;
        list->foot = node->prev;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    free(node);
}

void ll_free(list_t *list) {
    node_t *node = list->head;
    while (node != NULL) {
        node_t *data = node;
        node = node->next;
        free(data);
    }
}

void vec_init(vec_deque_t *vec) {
    vec->storage =
        (vec_item_t *)malloc(sizeof(vec_item_t) * VEC_INITIAL_CAPACITY);
    assert(vec->storage != NULL);
    vec->head = 0;
    vec->tail = 0;
    vec->capacity = VEC_INITIAL_CAPACITY;
}

/* this is quite a complicated implementation... originally I just used a plain
   old array to store this stuff, but decided I could save a tiny bit of memory
   by popping off the front of the queue as I go - also this data is
   all allocated in one contiguous region so it should be more efficient than
   a LinkedList given I don't need to insert/remove in the middle. */
void vec_push_back(vec_deque_t *vec, vec_item_t item) {
    vec->storage[vec->head] = item;
    vec->head = (vec->head + 1) % vec->capacity;

    /* buffer is full */
    if (vec->head == vec->tail) {
        /* now need to reallocate */
        size_t old_cap = vec->capacity;
        vec->capacity *= 2;
        vec->storage = (vec_item_t *)realloc(vec->storage, sizeof(vec_item_t) *
                                                               vec->capacity);
        /* if the head was at the end of the buffer, no need to move memory */
        if (vec->head == 0) {
            vec->head = old_cap;
            /* noop */
        } else if (vec->head < old_cap - vec->tail) {
            /* if the head was smaller than the tail, move it */
            memcpy(vec->storage + old_cap, vec->storage,
                   sizeof(vec_item_t) * vec->head);
            vec->head += old_cap;
        } else {
            /* if the tail was smaller than the head, move it to the end */
            size_t tail_size = old_cap - vec->tail;
            size_t next_tail = vec->capacity - tail_size;
            memcpy(vec->storage + next_tail, vec->storage + vec->tail,
                   sizeof(vec_item_t) * tail_size);
            vec->tail = next_tail;
        }
    }
}

bool vec_pop_front(vec_deque_t *vec, vec_item_t *item) {
    /* empty queue */
    if (vec->head == vec->tail) {
        return false;
    }

    *item = vec->storage[vec->tail];
    vec->tail = (vec->tail + 1) % vec->capacity;
    return true;
}

void vec_free(vec_deque_t *vec) {
    free(vec->storage);
    vec->storage = NULL;
}
