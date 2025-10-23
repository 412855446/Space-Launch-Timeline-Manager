\
        #ifndef EVENT_H
        #define EVENT_H

        #include <stdbool.h>

        #define ID_LEN 64
        #define DATE_LEN 11   /* "YYYY-MM-DD" + '\0' */
        #define VEHICLE_LEN 128
        #define MISSION_LEN 256
        #define SITE_LEN 256
        #define STATUS_LEN 32

        typedef struct {
            char id[ID_LEN];
            char date[DATE_LEN];
            char vehicle[VEHICLE_LEN];
            char mission[MISSION_LEN];
            char site[SITE_LEN];
            char status[STATUS_LEN];
        } Event;

        typedef struct Node {
            Event ev;
            struct Node *next;
        } Node;

        Node *create_node(const Event *ev);
        void free_list(Node *head);

        void insert_sorted(Node **head, const Event *ev); 
        int delete_by_id(Node **head, const char *id);
        Node *find_by_id(Node *head, const char *id);

        int load_csv(Node **head, const char *filename);
        int export_csv(Node *head, const char *filename);

        int validate_date(const char *date); 
        int valid_status(const char *status);
        void print_event(const Event *e);
        void print_list(Node *head);
        void find_range(Node *head, const char *start, const char *end);
        void find_keyword(Node *head, const char *keyword);
        void update_event_fields(Event *ev, int argc, char **argv);

        #endif 
