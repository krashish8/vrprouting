ADD_LIBRARY(common OBJECT
    postgres_connection.c
    e_report.c

    matrixRows_input.c
    get_check_data.c
    orders_input.c
    vehicles_input.c

    arrays_input.c
    time_msg.c
    pgr_alloc.cpp

    vroom/time_windows_input.c
    vroom/jobs_input.c
    # vroom/matrix_input.c
    # vroom/shipments_input.c
    # vroom/vehicles_input.c
    )
