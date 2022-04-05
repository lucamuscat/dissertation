#!/bin/bash
./enqueue_dequeue_runner_harness.sh blocking_linked_queue_ttas_lock ""
./enqueue_dequeue_runner_harness.sh nonblocking_valois_queue ""
./enqueue_dequeue_runner_harness.sh nonblocking_ms_queue ""
./enqueue_dequeue_runner_harness.sh nonblocking_baskets_queue ""