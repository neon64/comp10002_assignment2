# COMP10002 Assignment 2 (2019 Semester 2)

My solution for the second assignment.

The assignment had three main parts:

1. read in a grid size, path and obstacles from a file
2. visualise the grid and path with ASCII
3. repair a 'broken' path using a simple breadth-first search algorithm.

This required implementing a few data structures. I used a doubly-linked list
for the path, which may not have been strictly necessary, but was a good exercise (especially as it ended up being on the exam :D).

I also implemented a growable ring buffer inspired by Rust's [VecDeque](https://doc.rust-lang.org/std/collections/struct.VecDeque.html),
which was also not strictly necessary (I initially used a plain array), but was a bit of fun.

Due to the limitations of assignment submission, all the code had to be in a single file, `ass2.c`.

**See also:** my [course notes](https://github.com/neon64/unimelb_lecture_notes/blob/svgs/comp10002/README.md) from the semester.
