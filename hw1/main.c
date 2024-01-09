

struct movie {
    char *title;
    int year;
    char *languages[];
    double rating;
    struct movie *next;
};