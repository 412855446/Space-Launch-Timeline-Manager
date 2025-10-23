\
        #define _GNU_SOURCE
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        #include <ctype.h>
        #include "event.h"


        static void trim_newline(char *s) {
            if (!s) return;
            size_t len = strlen(s);
            if (len && s[len-1] == '\n') s[len-1] = '\0';
            if (len >= 2 && s[len-2] == '\r') s[len-2] = '\0';
        }

        Node *create_node(const Event *ev) {
            Node *n = malloc(sizeof(Node));
            if (!n) return NULL;
            n->ev = *ev;
            n->next = NULL;
            return n;
        }

        void free_list(Node *head) {
            Node *cur = head;
            while (cur) {
                Node *nx = cur->next;
                free(cur);
                cur = nx;
            }
        }

        static int date_cmp(const char *a, const char *b) {
            return strcmp(a, b);
        }

        void insert_sorted(Node **head, const Event *ev) {
            if (!head || !ev) return;
            Node *n = create_node(ev);
            if (!n) return;
            if (!*head || date_cmp(ev->date, (*head)->ev.date) < 0 ||
               (date_cmp(ev->date, (*head)->ev.date) == 0 && strcmp(ev->id, (*head)->ev.id) < 0)) {
                n->next = *head;
                *head = n;
                return;
            }
            Node *cur = *head;
            while (cur->next) {
                if (date_cmp(ev->date, cur->next->ev.date) < 0 ||
                   (date_cmp(ev->date, cur->next->ev.date) == 0 && strcmp(ev->id, cur->next->ev.id) < 0)) {
                    break;
                }
                cur = cur->next;
            }
            n->next = cur->next;
            cur->next = n;
        }

        int delete_by_id(Node **head, const char *id) {
            if (!head || !*head) return 0;
            Node *cur = *head, *prev = NULL;
            while (cur) {
                if (strcmp(cur->ev.id, id) == 0) {
                    if (!prev) *head = cur->next;
                    else prev->next = cur->next;
                    free(cur);
                    return 1;
                }
                prev = cur;
                cur = cur->next;
            }
            return 0;
        }

        Node *find_by_id(Node *head, const char *id) {
            Node *cur = head;
            while (cur) {
                if (strcmp(cur->ev.id, id) == 0) return cur;
                cur = cur->next;
            }
            return NULL;
        }

        int validate_date(const char *date) {
            if (!date) return 0;
            if (strlen(date) != 10) return 0;
            for (int i = 0; i < 10; ++i) {
                if (i==4 || i==7) {
                    if (date[i] != '-') return 0;
                } else {
                    if (!isdigit((unsigned char)date[i])) return 0;
                }
            }
            int y = (date[0]-'0')*1000 + (date[1]-'0')*100 + (date[2]-'0')*10 + (date[3]-'0');
            int m = (date[5]-'0')*10 + (date[6]-'0');
            int d = (date[8]-'0')*10 + (date[9]-'0');
            if (m < 1 || m > 12) return 0;
            if (d < 1) return 0;
            int mdays = 31;
            if (m==4||m==6||m==9||m==11) mdays = 30;
            else if (m==2) {
                int leap = ( (y%4==0 && y%100!=0) || (y%400==0) );
                mdays = leap ? 29 : 28;
            }
            if (d > mdays) return 0;
            return 1;
        }

        int valid_status(const char *status) {
            if (!status) return 0;
            const char *choices[] = {"Scheduled","Success","Failure","Delayed","Cancelled"};
            for (size_t i=0;i<sizeof(choices)/sizeof(choices[0]);++i) {
                if (strcmp(status, choices[i]) == 0) return 1;
            }
            return 0;
        }

        void print_event(const Event *e) {
            if (!e) return;
            printf("%s,%s,%s,%s,%s,%s\n",
                   e->id, e->date, e->vehicle, e->mission, e->site, e->status);
        }

        void print_list(Node *head) {
            Node *cur = head;
            while (cur) {
                print_event(&cur->ev);
                cur = cur->next;
            }
        }

        static int ci_substr(const char *hay, const char *needle) {
            if (!hay || !needle) return 0;
            size_t H = strlen(hay), N = strlen(needle);
            if (N == 0) return 1;
            for (size_t i = 0; i + N <= H; ++i) {
                size_t j;
                for (j = 0; j < N; ++j) {
                    if (tolower((unsigned char)hay[i+j]) != tolower((unsigned char)needle[j])) break;
                }
                if (j == N) return 1;
            }
            return 0;
        }

        void find_range(Node *head, const char *start, const char *end) {
            if (!validate_date(start) || !validate_date(end)) {
                printf("Invalid date(s). Use YYYY-MM-DD.\n");
                return;
            }
            Node *cur = head;
            while (cur) {
                if (strcmp(cur->ev.date, start) >= 0 && strcmp(cur->ev.date, end) <= 0) {
                    print_event(&cur->ev);
                }
                cur = cur->next;
            }
        }

        void find_keyword(Node *head, const char *keyword) {
            if (!keyword) return;
            Node *cur = head;
            while (cur) {
                if (ci_substr(cur->ev.mission, keyword) || ci_substr(cur->ev.vehicle, keyword)) {
                    print_event(&cur->ev);
                }
                cur = cur->next;
            }
        }

        void update_event_fields(Event *ev, int argc, char **argv) {
            for (int i = 0; i < argc; ++i) {
                char *eq = strchr(argv[i], '=');
                if (!eq) continue;
                *eq = '\\0';
                char *field = argv[i];
                char *value = eq + 1;
                if (strcmp(field, "date") == 0) {
                    if (validate_date(value)) strncpy(ev->date, value, DATE_LEN-1);
                } else if (strcmp(field, "vehicle") == 0) {
                    strncpy(ev->vehicle, value, VEHICLE_LEN-1);
                } else if (strcmp(field, "mission") == 0) {
                    strncpy(ev->mission, value, MISSION_LEN-1);
                } else if (strcmp(field, "site") == 0) {
                    strncpy(ev->site, value, SITE_LEN-1);
                } else if (strcmp(field, "status") == 0) {
                    if (valid_status(value)) strncpy(ev->status, value, STATUS_LEN-1);
                }
                *eq = '=';
            }
        }

        int load_csv(Node **head, const char *filename) {
            if (!filename || !head) return -1;
            FILE *f = fopen(filename, "r");
            if (!f) { perror("fopen"); return -1; }
            char *line = NULL;
            size_t cap = 0;
            ssize_t len;
            int added = 0, skipped = 0;
            if ((len = getline(&line, &cap, f)) == -1) { free(line); fclose(f); return 0; } /* header */
            while ((len = getline(&line, &cap, f)) != -1) {
                trim_newline(line);
                if (strlen(line) == 0) continue;
                char *fields[6] = {0};
                char *p = line;
                int idx = 0;
                char *tok;
                while ((tok = strsep(&p, ",")) != NULL && idx < 6) {
                    fields[idx++] = tok;
                }
                if (idx != 6) { skipped++; continue; }
                Event ev = {0};
                strncpy(ev.id, fields[0], ID_LEN-1);
                strncpy(ev.date, fields[1], DATE_LEN-1);
                strncpy(ev.vehicle, fields[2], VEHICLE_LEN-1);
                strncpy(ev.mission, fields[3], MISSION_LEN-1);
                strncpy(ev.site, fields[4], SITE_LEN-1);
                strncpy(ev.status, fields[5], STATUS_LEN-1);
                if (strlen(ev.id)==0 || !validate_date(ev.date) ||
                    strlen(ev.vehicle)==0 || strlen(ev.mission)==0 ||
                    strlen(ev.site)==0 || !valid_status(ev.status)) {
                    skipped++;
                    continue;
                }
                if (find_by_id(*head, ev.id)) { skipped++; continue; }
                insert_sorted(head, &ev);
                added++;
            }
            free(line);
            fclose(f);
            printf("Loaded: %d records, Skipped: %d invalid/duplicates.\n", added, skipped);
            return added;
        }

        int export_csv(Node *head, const char *filename) {
            if (!filename) return -1;
            FILE *f = fopen(filename, "w");
            if (!f) { perror("fopen"); return -1; }
            fprintf(f, "id,date,vehicle,mission,site,status\n");
            Node *cur = head;
            while (cur) {
                fprintf(f, "%s,%s,%s,%s,%s,%s\n",
                        cur->ev.id, cur->ev.date, cur->ev.vehicle,
                        cur->ev.mission, cur->ev.site, cur->ev.status);
                cur = cur->next;
            }
            fclose(f);
            printf("Exported list to %s\n", filename);
            return 0;
        }

        #define CMD_BUFSZ 1024

        static void print_help(void) {
            puts("Commands:");
            puts("  load <file>");
            puts("  add <id>,<date>,<vehicle>,<mission>,<site>,<status>");
            puts("  update <id> field=value ...");
            puts("  delete <id>");
            puts("  range <start_date> <end_date>");
            puts("  find <keyword>");
            puts("  export <file>");
            puts("  list");
            puts("  help");
            puts("  exit");
        }

        int main(void) {
            Node *head = NULL;
            char cmdline[CMD_BUFSZ];

            printf("sltm - Space Launch Timeline Manager (basic)\n");
            printf("Type 'help' for commands. Ctrl-C to quit.\n");

            while (1) {
                printf("sltm> ");
                if (!fgets(cmdline, sizeof(cmdline), stdin)) { putchar('\\n'); break; }
                trim_newline(cmdline);
                char *p = cmdline;
                while (*p && isspace((unsigned char)*p)) p++;
                if (*p == '\\0') continue;
                char *saveptr = NULL;
                char *cmd = strtok_r(p, " ", &saveptr);
                if (!cmd) continue;

                if (strcmp(cmd, "help") == 0) { print_help(); continue; }
                else if (strcmp(cmd, "load") == 0) {
                    char *file = strtok_r(NULL, " ", &saveptr);
                    if (!file) { printf("Usage: load <file>\\n"); continue; }
                    load_csv(&head, file);
                    continue;
                } else if (strcmp(cmd, "add") == 0) {
                    char *rest = strtok_r(NULL, "", &saveptr);
                    if (!rest) { printf("Usage: add <id>,<date>,<vehicle>,<mission>,<site>,<status>\\n"); continue; }
                    char *copy = strdup(rest);
                    char *p2 = copy;
                    char *fields[6] = {0};
                    int idx = 0;
                    char *tok;
                    while ((tok = strsep(&p2, ",")) != NULL && idx < 6) fields[idx++] = tok;
                    if (idx != 6) { printf("add: need 6 fields\\n"); free(copy); continue; }
                    Event ev = {0};
                    strncpy(ev.id, fields[0], ID_LEN-1);
                    strncpy(ev.date, fields[1], DATE_LEN-1);
                    strncpy(ev.vehicle, fields[2], VEHICLE_LEN-1);
                    strncpy(ev.mission, fields[3], MISSION_LEN-1);
                    strncpy(ev.site, fields[4], SITE_LEN-1);
                    strncpy(ev.status, fields[5], STATUS_LEN-1);
                    if (strlen(ev.id)==0 || !validate_date(ev.date) ||
                        strlen(ev.vehicle)==0 || strlen(ev.mission)==0 ||
                        strlen(ev.site)==0 || !valid_status(ev.status)) {
                        printf("add: invalid fields\\n");
                        free(copy);
                        continue;
                    }
                    if (find_by_id(head, ev.id)) { printf("add: id exists\\n"); free(copy); continue; }
                    insert_sorted(&head, &ev);
                    printf("Added %s\\n", ev.id);
                    free(copy);
                    continue;
                } else if (strcmp(cmd, "update") == 0) {
                    char *id = strtok_r(NULL, " ", &saveptr);
                    if (!id) { printf("Usage: update <id> field=value ...\\n"); continue; }
                    char *arg;
                    char *argv[16];
                    int argc = 0;
                    while ((arg = strtok_r(NULL, " ", &saveptr)) != NULL && argc < 16) argv[argc++] = arg;
                    if (argc == 0) { printf("update: need field=value\\n"); continue; }
                    Node *n = find_by_id(head, id);
                    if (!n) { printf("update: id not found\\n"); continue; }
                    Event copyev = n->ev;
                    update_event_fields(&copyev, argc, argv);
                    delete_by_id(&head, id);
                    insert_sorted(&head, &copyev);
                    printf("Updated %s\\n", id);
                    continue;
                } else if (strcmp(cmd, "delete") == 0) {
                    char *id = strtok_r(NULL, " ", &saveptr);
                    if (!id) { printf("Usage: delete <id>\\n"); continue; }
                    if (delete_by_id(&head, id)) printf("Deleted %s\\n", id);
                    else printf("delete: id not found\\n");
                    continue;
                } else if (strcmp(cmd, "range") == 0) {
                    char *s = strtok_r(NULL, " ", &saveptr);
                    char *e = strtok_r(NULL, " ", &saveptr);
                    if (!s || !e) { printf("Usage: range <start> <end>\\n"); continue; }
                    find_range(head, s, e);
                    continue;
                } else if (strcmp(cmd, "find") == 0) {
                    char *kw = strtok_r(NULL, "", &saveptr);
                    if (!kw) { printf("Usage: find <keyword>\\n"); continue; }
                    while (*kw && isspace((unsigned char)*kw)) kw++;
                    find_keyword(head, kw);
                    continue;
                } else if (strcmp(cmd, "export") == 0) {
                    char *file = strtok_r(NULL, " ", &saveptr);
                    if (!file) { printf("Usage: export <file>\\n"); continue; }
                    export_csv(head, file);
                    continue;
                } else if (strcmp(cmd, "list") == 0) {
                    print_list(head);
                    continue;
                } else if (strcmp(cmd, "exit") == 0) {
                    break;
                } else {
                    printf("Unknown command\\n");
                    continue;
                }
            }

            free_list(head);
            return 0;
        }
