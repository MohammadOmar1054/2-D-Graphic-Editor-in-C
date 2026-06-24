/*
 * 2D Graphics Editor
 * Uses a 2D character canvas to draw shapes using '*' and '_'
 * Supports: Circle, Rectangle, Line, Triangle
 * Operations: Add, Delete, Modify objects
 *
 * Author: [Your Name]
 * Course: Advanced C Programming
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─── Canvas dimensions ─── */
#define ROWS    25
#define COLS    70

/* ─── Max objects we can store ─── */
#define MAX_OBJECTS 50

/* ─── Shape type identifiers ─── */
#define SHAPE_CIRCLE    1
#define SHAPE_RECT      2
#define SHAPE_LINE      3
#define SHAPE_TRIANGLE  4

/* ─── The 2D canvas ─── */
char canvas[ROWS][COLS];

/* ─── A generic shape object ─── */
typedef struct {
    int  type;          /* SHAPE_* constant          */
    int  active;        /* 1 = alive, 0 = deleted    */
    char fill_char;     /* '*' or '_'                */

    /* Parameters interpreted per shape type */
    int  x1, y1;        /* primary point / center    */
    int  x2, y2;        /* secondary point / corner  */
    int  x3, y3;        /* third vertex (triangle)   */
    int  radius;        /* circle radius             */
} Object;

Object objects[MAX_OBJECTS];
int    obj_count = 0;   /* next free slot            */

/* ═══════════════════════════════════════════════════════
 *  Canvas helpers
 * ═══════════════════════════════════════════════════════ */

void canvas_clear(void)
{
    int r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

/* Plot a single point – bounds-checked */
void plot(int r, int c, char ch)
{
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

void canvas_display(void)
{
    int r, c;

    /* Top border */
    printf("+");
    for (c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (r = 0; r < ROWS; r++) {
        printf("|");
        for (c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    /* Bottom border */
    printf("+");
    for (c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}

/* ═══════════════════════════════════════════════════════
 *  Drawing primitives
 * ═══════════════════════════════════════════════════════ */

/* Bresenham's line algorithm */
void draw_line(int r1, int c1, int r2, int c2, char ch)
{
    int dr = abs(r2 - r1);
    int dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;
    int e2;

    while (1) {
        plot(r1, c1, ch);
        if (r1 == r2 && c1 == c2)
            break;
        e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

/*
 * Circle – midpoint circle algorithm.
 * We treat row as y and column as x, but columns are roughly
 * twice as wide visually, so we halve the horizontal radius
 * to keep circles looking round in a terminal.
 */
void draw_circle(int cr, int cc, int radius, char ch)
{
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    while (x <= y) {
        /* scale x by 2 to compensate for character aspect ratio */
        plot(cr - y, cc - 2*x, ch);
        plot(cr - y, cc + 2*x, ch);
        plot(cr + y, cc - 2*x, ch);
        plot(cr + y, cc + 2*x, ch);
        plot(cr - x, cc - 2*y, ch);
        plot(cr - x, cc + 2*y, ch);
        plot(cr + x, cc - 2*y, ch);
        plot(cr + x, cc + 2*y, ch);

        if (d < 0) {
            d += 2*x + 3;
        } else {
            d += 2*(x - y) + 5;
            y--;
        }
        x++;
    }
}

/* Axis-aligned rectangle (outline only) */
void draw_rectangle(int r1, int c1, int r2, int c2, char ch)
{
    int r, c;

    /* Make sure r1 <= r2, c1 <= c2 */
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }

    for (c = c1; c <= c2; c++) {
        plot(r1, c, ch);
        plot(r2, c, ch);
    }
    for (r = r1; r <= r2; r++) {
        plot(r, c1, ch);
        plot(r, c2, ch);
    }
}

/* Triangle – three vertices connected by lines */
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch)
{
    draw_line(r1, c1, r2, c2, ch);
    draw_line(r2, c2, r3, c3, ch);
    draw_line(r3, c3, r1, c1, ch);
}

/* ═══════════════════════════════════════════════════════
 *  Render all active objects onto a fresh canvas
 * ═══════════════════════════════════════════════════════ */

void render_all(void)
{
    int i;
    canvas_clear();

    for (i = 0; i < obj_count; i++) {
        if (!objects[i].active)
            continue;

        Object *o = &objects[i];

        switch (o->type) {
        case SHAPE_CIRCLE:
            draw_circle(o->x1, o->y1, o->radius, o->fill_char);
            break;
        case SHAPE_RECT:
            draw_rectangle(o->x1, o->y1, o->x2, o->y2, o->fill_char);
            break;
        case SHAPE_LINE:
            draw_line(o->x1, o->y1, o->x2, o->y2, o->fill_char);
            break;
        case SHAPE_TRIANGLE:
            draw_triangle(o->x1, o->y1, o->x2, o->y2, o->x3, o->y3, o->fill_char);
            break;
        default:
            break;
        }
    }
}

/* ═══════════════════════════════════════════════════════
 *  Object management
 * ═══════════════════════════════════════════════════════ */

/* Returns index of newly added slot, or -1 if full */
int add_object(void)
{
    if (obj_count >= MAX_OBJECTS) {
        printf("Canvas is full (max %d objects).\n", MAX_OBJECTS);
        return -1;
    }
    memset(&objects[obj_count], 0, sizeof(Object));
    objects[obj_count].active = 1;
    return obj_count++;
}

void list_objects(void)
{
    int i;
    int found = 0;

    printf("\n%-4s %-10s %-6s  Details\n", "ID", "Shape", "Char");
    printf("%-4s %-10s %-6s  -------\n", "--", "-----", "----");

    for (i = 0; i < obj_count; i++) {
        if (!objects[i].active)
            continue;
        found = 1;
        Object *o = &objects[i];
        const char *name = "?";

        switch (o->type) {
        case SHAPE_CIRCLE:   name = "Circle";    break;
        case SHAPE_RECT:     name = "Rectangle"; break;
        case SHAPE_LINE:     name = "Line";      break;
        case SHAPE_TRIANGLE: name = "Triangle";  break;
        }

        printf("%-4d %-10s  '%c'   ", i, name, o->fill_char);

        switch (o->type) {
        case SHAPE_CIRCLE:
            printf("center=(%d,%d) r=%d", o->x1, o->y1, o->radius);
            break;
        case SHAPE_RECT:
            printf("(%d,%d) -> (%d,%d)", o->x1, o->y1, o->x2, o->y2);
            break;
        case SHAPE_LINE:
            printf("(%d,%d) -> (%d,%d)", o->x1, o->y1, o->x2, o->y2);
            break;
        case SHAPE_TRIANGLE:
            printf("(%d,%d) (%d,%d) (%d,%d)",
                   o->x1, o->y1, o->x2, o->y2, o->x3, o->y3);
            break;
        }
        putchar('\n');
    }

    if (!found)
        printf("  (no objects yet)\n");
    putchar('\n');
}

int get_valid_id(const char *prompt)
{
    int id;
    printf("%s", prompt);
    scanf("%d", &id);
    if (id < 0 || id >= obj_count || !objects[id].active) {
        printf("Invalid or deleted object ID.\n");
        return -1;
    }
    return id;
}

char get_fill_char(void)
{
    char ch;
    printf("Fill character ('*' or '_'): ");
    scanf(" %c", &ch);
    if (ch != '*' && ch != '_') {
        printf("Unknown character – defaulting to '*'.\n");
        ch = '*';
    }
    return ch;
}

/* ═══════════════════════════════════════════════════════
 *  Add-shape menus
 * ═══════════════════════════════════════════════════════ */

void add_circle(void)
{
    int idx = add_object();
    if (idx < 0) return;

    Object *o = &objects[idx];
    o->type = SHAPE_CIRCLE;
    o->fill_char = get_fill_char();

    printf("Center row (0-%d): ", ROWS-1);  scanf("%d", &o->x1);
    printf("Center col (0-%d): ", COLS-1);  scanf("%d", &o->y1);
    printf("Radius: ");                       scanf("%d", &o->radius);

    printf("Circle added with ID %d.\n", idx);
}

void add_rectangle(void)
{
    int idx = add_object();
    if (idx < 0) return;

    Object *o = &objects[idx];
    o->type = SHAPE_RECT;
    o->fill_char = get_fill_char();

    printf("Top-left  row (0-%d): ", ROWS-1);   scanf("%d", &o->x1);
    printf("Top-left  col (0-%d): ", COLS-1);   scanf("%d", &o->y1);
    printf("Bot-right row (0-%d): ", ROWS-1);   scanf("%d", &o->x2);
    printf("Bot-right col (0-%d): ", COLS-1);   scanf("%d", &o->y2);

    printf("Rectangle added with ID %d.\n", idx);
}

void add_line(void)
{
    int idx = add_object();
    if (idx < 0) return;

    Object *o = &objects[idx];
    o->type = SHAPE_LINE;
    o->fill_char = get_fill_char();

    printf("Start row (0-%d): ", ROWS-1);  scanf("%d", &o->x1);
    printf("Start col (0-%d): ", COLS-1);  scanf("%d", &o->y1);
    printf("End   row (0-%d): ", ROWS-1);  scanf("%d", &o->x2);
    printf("End   col (0-%d): ", COLS-1);  scanf("%d", &o->y2);

    printf("Line added with ID %d.\n", idx);
}

void add_triangle(void)
{
    int idx = add_object();
    if (idx < 0) return;

    Object *o = &objects[idx];
    o->type = SHAPE_TRIANGLE;
    o->fill_char = get_fill_char();

    printf("Vertex 1 row: ");  scanf("%d", &o->x1);
    printf("Vertex 1 col: ");  scanf("%d", &o->y1);
    printf("Vertex 2 row: ");  scanf("%d", &o->x2);
    printf("Vertex 2 col: ");  scanf("%d", &o->y2);
    printf("Vertex 3 row: ");  scanf("%d", &o->x3);
    printf("Vertex 3 col: ");  scanf("%d", &o->y3);

    printf("Triangle added with ID %d.\n", idx);
}

/* ═══════════════════════════════════════════════════════
 *  Delete / Modify
 * ═══════════════════════════════════════════════════════ */

void delete_object(void)
{
    list_objects();
    int id = get_valid_id("Enter ID to delete: ");
    if (id < 0) return;
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
}

/*
 * Modify: we re-use the same add_* logic but overwrite the
 * existing slot instead of creating a new one.
 */
void modify_object(void)
{
    list_objects();
    int id = get_valid_id("Enter ID to modify: ");
    if (id < 0) return;

    Object *o = &objects[id];
    int old_type = o->type;
    int new_type;

    printf("New shape type (1=Circle 2=Rectangle 3=Line 4=Triangle, 0=keep %d): ", old_type);
    scanf("%d", &new_type);
    if (new_type >= 1 && new_type <= 4)
        o->type = new_type;

    o->fill_char = get_fill_char();

    switch (o->type) {
    case SHAPE_CIRCLE:
        printf("Center row: ");  scanf("%d", &o->x1);
        printf("Center col: ");  scanf("%d", &o->y1);
        printf("Radius:     ");  scanf("%d", &o->radius);
        break;
    case SHAPE_RECT:
        printf("Top-left  row: ");  scanf("%d", &o->x1);
        printf("Top-left  col: ");  scanf("%d", &o->y1);
        printf("Bot-right row: ");  scanf("%d", &o->x2);
        printf("Bot-right col: ");  scanf("%d", &o->y2);
        break;
    case SHAPE_LINE:
        printf("Start row: ");  scanf("%d", &o->x1);
        printf("Start col: ");  scanf("%d", &o->y1);
        printf("End   row: ");  scanf("%d", &o->x2);
        printf("End   col: ");  scanf("%d", &o->y2);
        break;
    case SHAPE_TRIANGLE:
        printf("V1 row: ");  scanf("%d", &o->x1);
        printf("V1 col: ");  scanf("%d", &o->y1);
        printf("V2 row: ");  scanf("%d", &o->x2);
        printf("V2 col: ");  scanf("%d", &o->y2);
        printf("V3 row: ");  scanf("%d", &o->x3);
        printf("V3 col: ");  scanf("%d", &o->y3);
        break;
    }

    printf("Object %d updated.\n", id);
}

/* ═══════════════════════════════════════════════════════
 *  Save canvas to a text file
 * ═══════════════════════════════════════════════════════ */

void save_canvas(void)
{
    char filename[128];
    printf("Enter filename to save (e.g. picture.txt): ");
    scanf("%127s", filename);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return;
    }

    int r, c;
    fprintf(fp, "+");
    for (c = 0; c < COLS; c++) fputc('-', fp);
    fprintf(fp, "+\n");

    for (r = 0; r < ROWS; r++) {
        fputc('|', fp);
        for (c = 0; c < COLS; c++) fputc(canvas[r][c], fp);
        fprintf(fp, "|\n");
    }

    fprintf(fp, "+");
    for (c = 0; c < COLS; c++) fputc('-', fp);
    fprintf(fp, "+\n");

    fclose(fp);
    printf("Canvas saved to '%s'.\n", filename);
}

/* ═══════════════════════════════════════════════════════
 *  Main menu
 * ═══════════════════════════════════════════════════════ */

void print_menu(void)
{
    printf("\n╔══════════════════════════════╗\n");
    printf("║    2D Graphics Editor        ║\n");
    printf("╠══════════════════════════════╣\n");
    printf("║  1. Add Circle               ║\n");
    printf("║  2. Add Rectangle            ║\n");
    printf("║  3. Add Line                 ║\n");
    printf("║  4. Add Triangle             ║\n");
    printf("║  5. Delete Object            ║\n");
    printf("║  6. Modify Object            ║\n");
    printf("║  7. List Objects             ║\n");
    printf("║  8. Display Canvas           ║\n");
    printf("║  9. Save Canvas to File      ║\n");
    printf("║  0. Exit                     ║\n");
    printf("╚══════════════════════════════╝\n");
    printf("Choice: ");
}

int main(void)
{
    int choice;

    canvas_clear();
    printf("Welcome to the 2D Graphics Editor!\n");
    printf("Canvas: %d rows x %d cols\n", ROWS, COLS);
    printf("Shapes are drawn with '*' or '_'\n");

    while (1) {
        print_menu();
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            add_circle();
            render_all();
            canvas_display();
            break;
        case 2:
            add_rectangle();
            render_all();
            canvas_display();
            break;
        case 3:
            add_line();
            render_all();
            canvas_display();
            break;
        case 4:
            add_triangle();
            render_all();
            canvas_display();
            break;
        case 5:
            delete_object();
            render_all();
            canvas_display();
            break;
        case 6:
            modify_object();
            render_all();
            canvas_display();
            break;
        case 7:
            list_objects();
            break;
        case 8:
            render_all();
            canvas_display();
            break;
        case 9:
            render_all();
            save_canvas();
            break;
        case 0:
            printf("Goodbye!\n");
            return 0;
        default:
            printf("Invalid choice, please try again.\n");
        }
    }

    return 0;
}
