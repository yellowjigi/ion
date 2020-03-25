/********************************************************
    Authors:
    Lorenzo Mustich, lorenzo.mustich@studio.unibo.it
    Carlo Caini (DTNproxy project supervisor), carlo.caini@unibo.it

    License:
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Copyright (c) 2018, Alma Mater Studiorum, University of Bologna

 ********************************************************/

/*
 * bundle_receiver.c
 *
 * This file contains code for receving from
 * a DTN sending application
 *
 */

#include "proxy_thread.h"
#include "bundle_header_utility.h"
#include "debugger.h"
#include "utility.h"

/**
 * static variables
 */
static al_bp_bundle_object_t bundle;

/**
 * function interfaces
 */
al_bp_error_t read_dtnperf_header(char* filename);
//void handlerBundleReceiver(int signal);

/**
 * Thread code
 */
void * bundleReceiving(void * arg) {
	al_bp_extB_error_t utility_error;
	al_bp_error_t error;
	bundle_to_tcp_inf_t * proxy_inf = (bundle_to_tcp_inf_t *) arg;

	al_bp_bundle_payload_location_t location = BP_PAYLOAD_FILE;
	al_bp_endpoint_id_t source_eid;

	char * file_name_payload;
	uint file_name_payload_len;
	char filename[FILE_NAME];

	int fd, fdNew;
	char char_read;

	int index = 0;

	opendir(DTN_TCP_DIR);
	if(errno == ENOENT) {
		debug_print(proxy_inf->debug_level, "[DEBUG] Creating %s\n", DTN_TCP_DIR);
		mkdir(DTN_TCP_DIR, 0700);
	}

//	signal(SIGUSR1, handlerBundleReceiver);

	while(*(proxy_inf->is_running)) {
		sem_wait(&proxy_inf->bundleRecv);

		//Create bundle
		utility_error = al_bp_bundle_create(&bundle);
		if (utility_error != BP_EXTB_SUCCESS) {
			error_print("Error in al_bp_bundle_create() (%s)\n", al_bp_strerror(utility_error));
			al_bp_bundle_free(&bundle);
			continue;
		}
		printf("Waiting for a bundle...\n");

		//Receive a bundle
		if(*(proxy_inf->is_running) == 0)
			break;

		utility_error = al_bp_extB_receive(proxy_inf->rd_receive, bundle, location, -1);
		if (utility_error != BP_EXTB_SUCCESS) {
			error_print("Error in al_bp_extB_receive_bundle() (%s)\n", al_bp_strerror(utility_error));
			al_bp_bundle_free(&bundle);

			set_is_running_to_false(proxy_inf->mutex, proxy_inf->is_running);
//			kill(proxy_inf->tid_snd, SIGUSR1);
			break;
		}
		printf("Bundle received\n");

		if(*(proxy_inf->is_running) == 0)
			break;


		utility_error = al_bp_bundle_get_payload_file(bundle, &file_name_payload, &file_name_payload_len);
		if (utility_error != BP_EXTB_SUCCESS) {
			error_print("Error in al_bp_bundle_get_file_name_payload_file() (%s)\n", al_bp_strerror(utility_error));
			al_bp_bundle_free(&bundle);
			continue;
		}

		utility_error = al_bp_bundle_get_source(bundle, &source_eid);
		if (utility_error != BP_EXTB_SUCCESS) {
			error_print("Error in al_bp_bundle_get_source() (%s)\n", al_bp_strerror(utility_error));
			al_bp_bundle_free(&bundle);
			continue;
		}
		debug_print(proxy_inf->debug_level,
				"[DEBUG] file_name: %s EID_src: %s\n", basename(file_name_payload), source_eid.uri);

		if(proxy_inf->options == 'm') {
			fd = open(file_name_payload, O_RDONLY);
			if (fd < 0) {
				error_print("Opening file %s failed (%s)\n", file_name_payload, strerror(errno));
				al_bp_bundle_free(&bundle);
				continue;
			}

			strcpy(filename, DTN_TCP_DIR);
			strcat(filename, &source_eid.uri[4]);
			strcat(filename, "_");
			strcat(filename, &file_name_payload[5]);
			sprintf(filename, "%s_%d", filename, index);

			fdNew = open(filename, O_WRONLY | O_CREAT, 0700);
			if (fd < 0) {
				error_print("Reading file %s failed (%s)\n", file_name_payload, strerror(errno));
				al_bp_bundle_free(&bundle);
				close(fd);
				continue;
			}

			if(*(proxy_inf->is_running) == 0)
				break;

			while(read(fd, &char_read, sizeof(char)) > 0) {
				write(fdNew, &char_read, sizeof(char));
			}
			close(fd);
			close(fdNew);

			if(*(proxy_inf->is_running) == 0)
				break;

			strcpy(proxy_inf->buffer[index], filename);
		}
		else { //DTNperf compatibility
			HEADER_TYPE bundle_header;
			if (get_dtnperf_bundle_header_and_options(&bundle, &bundle_header) < 0) {
				error_print("Error in getting bundle header and options\n");
				al_bp_bundle_free(&bundle);
				continue;
			}

			error = read_dtnperf_header(proxy_inf->buffer[index]);
			if(error != BP_SUCCESS) {
				error_print("Error in reading dtnperf header (%s)\n", al_bp_strerror(error));
				al_bp_bundle_free(&bundle);
				continue;
			}
		}

		debug_print(proxy_inf->debug_level,
				"[DEBUG] index: %d %s\n", index, proxy_inf->buffer[index]);
		index = (index + 1) % MAX_NUM_FILE;

		utility_error = al_bp_bundle_free(&bundle);
		if (utility_error != BP_EXTB_SUCCESS) {
			error_print("Error in al_bp_bundle_free() (%s)\n", al_bp_strerror(utility_error));
			al_bp_bundle_free(&bundle);

			set_is_running_to_false(proxy_inf->mutex, proxy_inf->is_running);
			//kill(proxy_inf->tid_snd, SIGUSR1);
			break;
		}

		strcpy(filename, "");

		sem_post(&proxy_inf->tcpSnd);
	}//for
//
//	printf("BundleSender: i'm here\n");
//
//	int value;
//	sem_getvalue(&proxy_inf->tcpSnd, &value);
//	if(value == 0)
//		sem_post(&proxy_inf->tcpSnd);
//
//	pthread_exit((void *) EXIT_FAILURE);

	return NULL;
}

//void handlerBundleReceiver(int signal) {
//
//}

/**
 * Function for dtnperf compability
 */
al_bp_error_t read_dtnperf_header(char * filename) {
	FILE *pl_stream;
	char *transfer;
	int transfer_len;
	u32_t pl_size;

	//Get info about bundle size
	al_bp_bundle_get_payload_size(bundle, &pl_size);

	if (dtnperf_open_payload_stream_read(bundle, &pl_stream) < 0)
	{
		error_print("Error in opening file transfer bundle (%s)\n", strerror(errno));
		return BP_ERRBASE;
	}

	transfer_len = HEADER_SIZE + BUNDLE_OPT_SIZE+sizeof(al_bp_timeval_t);
	transfer = (char*) malloc(transfer_len);
	memset(transfer, 0, transfer_len);

	if (fread(transfer, transfer_len, 1, pl_stream) != 1 && ferror(pl_stream)!=0)
	{
		error_print("Error in processing file transfer bundle (%s)\n", strerror(errno));
		return BP_ERRBASE;
	}
	free(transfer);

	fseek(pl_stream, BUNDLE_CRC_SIZE, SEEK_CUR);

	transfer_len = pl_size-transfer_len-BUNDLE_CRC_SIZE;
	transfer = (char*) malloc(transfer_len);
	memset(transfer, 0, transfer_len);

	if (fread(transfer, transfer_len, 1, pl_stream) != 1 && ferror(pl_stream)!=0)
	{
		error_print("Error in processing file transfer bundle (%s)\n", strerror(errno));
		return BP_ERRBASE;
	}

	free(transfer);
	dtnperf_close_payload_stream_read(pl_stream);

	dtnperf_process_incoming_file(filename, &bundle);

	return BP_SUCCESS;
}



