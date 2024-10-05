// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;

pthread_mutex_t graph_lock;

struct timespec ts = {
	.tv_nsec = 1000000,
	.tv_sec = 0
};

/* Process node task-ified */
static void task_action(void *arg)
{
	unsigned int idx = *((unsigned int *)arg);
	os_node_t *node = graph->nodes[idx];

	nanosleep(&ts, NULL);

	pthread_mutex_lock(&graph_lock);

	if (graph->visited[idx] == DONE) {
		pthread_mutex_unlock(&graph_lock);
		return;
	}

	/* Visit node */
	sum += node->info;
	graph->visited[idx] = DONE;

	for (size_t i = 0; i < node->num_neighbours; i++) {
		if (graph->visited[node->neighbours[i]] == NOT_VISITED) {
			graph->visited[node->neighbours[i]] = PROCESSING;

			pthread_mutex_unlock(&graph_lock);

			enqueue_task(tp, create_task(task_action, &node->neighbours[i], NULL));

			pthread_mutex_lock(&graph_lock);
		}
	}

	pthread_mutex_unlock(&graph_lock);
}

/* Main thread entry point */
static void process_node(unsigned int idx)
{
	os_node_t *node = graph->nodes[idx];

	sum += node->info;

	pthread_mutex_lock(&graph_lock);

	graph->visited[idx] = DONE;

	for (size_t i = 0; i < node->num_neighbours; i++) {
		if (graph->visited[node->neighbours[i]] == NOT_VISITED) {
			graph->visited[node->neighbours[i]] = PROCESSING;

			pthread_mutex_unlock(&graph_lock);

			enqueue_task(tp, create_task(task_action, &node->neighbours[i], NULL));

			pthread_mutex_lock(&graph_lock);
		}
	}

	pthread_mutex_unlock(&graph_lock);
}

static void free_graph(os_graph_t *graph)
{
	for (size_t i = 0; i < graph->num_nodes; i++) {
		free(graph->nodes[i]->neighbours);
		free(graph->nodes[i]);
	}

	free(graph->nodes);
	free(graph->visited);
	free(graph);
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	DIE(pthread_mutex_init(&graph_lock, NULL) != 0, MUTEX_INIT_FAIL);

	graph = create_graph_from_file(input_file);

	fclose(input_file);

	tp = create_threadpool(NUM_THREADS);
	process_node(0);
	wait_for_completion(tp);
	destroy_threadpool(tp);

	printf("%d", sum);

	pthread_mutex_destroy(&graph_lock);
	free_graph(graph);

	return 0;
}
