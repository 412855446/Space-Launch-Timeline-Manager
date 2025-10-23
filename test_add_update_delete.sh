\
        #!/bin/bash
        ./sltm <<EOF
        load sample_launches.csv
        add MY001,2025-06-15,Falcon 9,Test Add Mission,CCSFS SLC-40,Scheduled
        find Test
        update MY001 status=Success mission=Test_Add_Updated
        find Test_Add_Updated
        delete MY001
        find Test_Add_Updated
        exit
        EOF
