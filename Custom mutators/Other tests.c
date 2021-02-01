size_t incremental(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;
	u8 *mutated_out = 1;

	//16 bit bruteforce stage
	if(data->stage == 0){

			if(data->afl->stage_cur == 0){

				  DIR * dir = opendir(data->afl->in_dir);

				  if(dir){

					  struct dirent* in_file;

					  while ((in_file = readdir(dir))) {

					          /* On linux/Unix we don't want current and parent directories
					           * On windows machine too, thanks Greg Hewgill
					           */
					          if (!strcmp (in_file->d_name, "."))
					              continue;
					          if (!strcmp (in_file->d_name, ".."))
					              continue;
					          /* Open directory entry file for common operation */
					          /* TODO : change permissions to meet your need! */
					          mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

					          u_char tmp[512];
					          strcpy(tmp, data->afl->in_dir);
					          strcat(tmp, in_file->d_name);

					          int fd = open(tmp, O_WRONLY|O_TRUNC, mode);

					          if(fd){
					        	  uint tmp = 0;
					        	  write(fd, &tmp, 2);
					        	  close(fd);
					          }
					  }
				  }

				data->max_steps = 256*256;
				data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
			}

			data->afl->stage_max = data->max_steps;

			mutated_size = 2;

			mutated_out = maybe_grow((void**)&buf, &buf_size, mutated_size);
			if (!mutated_out) {
				*out_buf = NULL;
				perror("custom mutator allocation (maybe_grow)");
				return 0;
			}

			mutated_out[0] = data->index_i;
			mutated_out[1] = data->index_j;

			data->index_j++;

			if(data->index_j >= 256){
				data->index_j = 0;
				data->index_i++;
			}

			if(data->afl->stage_cur == data->afl->stage_max-1){
				//printf("yeaahhh\n");
				data->stage++;
				data->cur_length = 2;
				data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
				data->next_level = 1;
			}



	//Incremental stage
	} else if(data->stage == 1){

		if(data->next_level){
			data->next_level = 0;
			data->cur_length++;
			zero_add(data);
			reset_values(data, data->cur_length);
			//mutated_size = 0;
			//data->afl->stage_max = 1;
			//goto end;
		}

		if(data->remaining == 0){
			mutated_size = 0;
			data->afl->stage_max = 1;
			data->next_level = 1;
			goto end;

		}else if(data->afl->queue_cur->trim_done < 1){
			mutated_size = 0;
			goto end;
		}

		if(data->afl->stage_cur == 0){
			data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
			data->max_steps = 99;
			data->index_i = data->cur_length - 1;
			data->byte_count_1 = 0x09;
		}

		data->afl->pending_not_fuzzed = data->remaining;
		data->afl->stage_max = data->max_steps;

		mutated_size = data->cur_length;

		mutated_out = maybe_grow((void**)&buf, &buf_size, mutated_size);
		if (!mutated_out) {
			*out_buf = NULL;
			perror("custom mutator allocation (maybe_grow)");
			return 0;
		}

		mutated_out[data->index_i] = data->byte_count_1;

		if(data->byte_count_1 == 0x0A)
			data->byte_count_1 = 0x0D;
		else if(data->byte_count_1 == 0x0D)
			data->byte_count_1 = 0x20;
		else
			data->byte_count_1++;

		if(data->afl->stage_cur >= data->max_steps-1){
			data->afl->queue_cur->trim_done = 0;
			data->remaining--;
			data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
		}

		goto end;

		return brute_8bits(data, buf, buf_size, out_buf, max_size);

		if(data->afl->queue_cur->trim_done > 0){
			mutated_size = 0;
			data->afl->stage_max = 1;
			goto end;
		}

		u_int n_lines;

		if(data->afl->stage_cur == 0){

			n_lines = 1;
			for(int i=0; i<buf_size; i++){
				if(buf[i] == 0x0A){
					n_lines++;
				}
			}

			data->max_steps = n_lines * data->num_lines;

			data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
			data->index_i = 0;
			data->index_j = 0;
		}

		data->afl->stage_max = data->max_steps;
		data->afl->havoc_max_mult = 0;

		mutated_size = buf_size <= max_size ? buf_size : max_size;

		// |0|1|2|3|4|5|

		uint ptr = data->index_i;

		while(ptr < buf_size && buf[ptr] != 0x0A){
			ptr++;
		}

		uint post_size = buf_size - ptr;

		uint new_size = data->index_i + lines[data->index_j].size + post_size;
		uint8_t *new_array = malloc(new_size);

		uint tmp = 0;

		if(data->index_i){
			memcpy(&new_array[tmp], &buf[0], data->index_i);
			tmp += data->index_i;
		}

		memcpy(&new_array[tmp], lines[data->index_j].p, lines[data->index_j].size);
		tmp += lines[data->index_j].size;

		if(post_size){
			memcpy(&new_array[tmp], &buf[ptr], post_size);
		}

		//printf("%d %d\n", new_size, mutated_size);
		if(new_size > mutated_size){
			mutated_size = new_size;
		}

		mutated_out = maybe_grow((void**)&buf, &buf_size, mutated_size);
		if (!mutated_out) {
			*out_buf = NULL;
			perror("custom mutator allocation (maybe_grow)");
			return 0;
		}

		mutated_size = new_size;

		memcpy(mutated_out, new_array, new_size);

		free(new_array);

		data->index_j++;

		if(data->index_j >= data->num_lines){
			data->index_j = 0;
			data->index_i = ptr+1;
		}

		if(data->afl->stage_cur == data->afl->stage_max-1){
			//printf("yeaahhh\n");
			data->afl->queue_cur->trim_done = 1;
			data->remaining--;
			data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
		}
	}

	end:
	*out_buf = mutated_out;
	return mutated_size;
}

size_t generator_8bits(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;

	if(data->new_level){
		//data->remaining = data->afl->pending_not_fuzzed;
		data->remaining = inputs_n_bytes(data->index_i, data);
		data->new_level = 0;
	}

	if(data->afl->queue_cur->was_fuzzed == 2 || (data->index_i > 1 && data->index_i != buf_size)){
		mutated_size = 0;
		data->afl->stage_max = 1;
		goto end;
	}

	if(data->afl->stage_cur == 0){
		data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
		if(data->afl->cur_depth == 1){
			data->index_i = 1;

		}
		data->byte_count_1 = 0;
	}

	data->afl->stage_max = 256;
	data->afl->havoc_max_mult = 0;


	// Make sure that the packet size does not exceed the maximum size expected by the fuzzer
	mutated_size = data->index_i+1;

	// maybe_grow is optimized to be quick for reused buffers.
	u8 *mutated_out = buf;
	//memcpy(mutated_out, buf, data->index_i);
	if (!mutated_out) {
		*out_buf = NULL;
		perror("custom mutator allocation (maybe_grow)");
		return 0;            /* afl-fuzz will very likely error out after this. */
	}

	mutated_out[data->index_i] = data->byte_count_1;

	++data->byte_count_1;


	if(data->afl->stage_cur >= 255){
		//printf("yeaahhh\n");
		data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
		data->remaining--;
		data->afl->queue_cur->was_fuzzed = 2;
	}

	if(data->remaining == 0){
		data->index_i++;
		data->new_level = 1;
	}


	end:
	*out_buf = mutated_out;
	return mutated_size;
}

size_t back_8bits(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;

	if(data->afl->queue_cur->was_fuzzed){
		mutated_size = 0;
		data->afl->stage_max = 1;
		goto end;
	}

	if(data->afl->stage_cur == 0){
		data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
		data->index_i = data->afl->cur_depth-1;
		data->byte_count_1 = 0;
	}

	data->afl->stage_max = 256;
	data->afl->havoc_max_mult = 0;


	// Make sure that the packet size does not exceed the maximum size expected by the fuzzer
	mutated_size = buf_size <= max_size ? buf_size : max_size;


	// maybe_grow is optimized to be quick for reused buffers.
	u8 *mutated_out = maybe_grow((void**)&buf, &buf_size, mutated_size);
	if (!mutated_out) {

		*out_buf = NULL;
		perror("custom mutator allocation (maybe_grow)");
		return 0;            /* afl-fuzz will very likely error out after this. */
	}

	mutated_out[data->index_i] = data->byte_count_1;

	++data->byte_count_1;


	if(data->afl->stage_cur >= 255){
		//printf("yeaahhh\n");
		data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
	}


	end:
	*out_buf = mutated_out;
	return mutated_size;
}

size_t afl_custom_fuzz(my_mutator_t *data, uint8_t *buf, size_t buf_size,
                       u8 **out_buf, uint8_t *add_buf, size_t add_buf_size,
                       size_t max_size) {

	if(data->brute_level == 40)
		return incremental(data, buf, buf_size, out_buf, max_size);

	if(data->brute_level == 10)
		return back_8bits(data, buf, buf_size, out_buf, max_size);

	if(data->brute_level == 11)
		return generator_8bits(data, buf, buf_size, out_buf, max_size);
	
}