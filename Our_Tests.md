\
        # Our Tests and Expected Outputs

        ## 1) test_load.sh
        - Expect: "Loaded: 32 records, Skipped: 0 invalid/duplicates."
        - `list` prints 32 records in chronological order.

        ## 2) test_invalid_load.sh
        - Expect: Loaded: 1 records, Skipped: 4 invalid/duplicates.

        ## 3) test_add_update_delete.sh
        - Add MY001, find it, update fields, delete it. Confirm messages printed.

        ## 4) test_range_find_export.sh
        - Prints launches between 2025-06-01 and 2025-07-31; finds Falcon entries; exports out_export.csv.

        ## 5) test_update_date_reposition.sh
        - Adding TEMP1 then updating its date repositions it in sorted order.

        ## 6) test_duplicates_and_errors.sh
        - Duplicate add and bad-date add are rejected; update/delete of missing id are handled.
