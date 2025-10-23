\
        #!/bin/bash
        ./sltm <<EOF
        load sample_launches.csv
        add SLV001,2025-06-01,Falcon 9,Duplicate Test,CCSFS SLC-40,Scheduled
        add BAD1,2025-06-xx,Falcon 9,Bad Date,CCSFS SLC-40,Scheduled
        update NOID status=Success
        delete NOID
        exit
        EOF
