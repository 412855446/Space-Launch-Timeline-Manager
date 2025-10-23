\
        #!/bin/bash
        ./sltm <<EOF
        load sample_launches.csv
        range 2025-06-01 2025-07-31
        find Falcon
        export out_export.csv
        exit
        EOF
