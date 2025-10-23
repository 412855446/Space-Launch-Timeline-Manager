\
        #!/bin/bash
        ./sltm <<EOF
        load sample_launches.csv
        add TEMP1,2025-09-01,TestRocket,TestMission,TestSite,Scheduled
        list
        update TEMP1 date=2025-03-05
        list
        delete TEMP1
        exit
        EOF
