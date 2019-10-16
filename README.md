current issue - initial cell COULD be part of path afterwards

- must check for whether a cell is part of the later path, not by a linear O(1) lookup in the grid, but rather by traversing the linked-list in O(n) time. Ouch. - no this isn't required!!!