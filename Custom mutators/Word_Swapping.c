


size_t lines_mix(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size, char separator){

	size_t mutated_size;
	u8 *mutated_out = 1;

	//Initial stage
	if(data->stage == 0){

		if(data->afl->pending_not_fuzzed == 0){
			data->stage++;
			data->remaining = data->afl->queued_paths;

		}else{

			mutated_size = 0;

			if(data->afl->queue_cur->was_fuzzed){
				goto end;
			}

			data->afl->stage_max = 1;

			u_int ptr = 0;
			for(int i=0; i<buf_size; i++){

				if(buf[i] == separator){

					u_int length = i-ptr;

					if(length > 0){

						if(!repeated(data, &buf[ptr], length)){

							lines[data->num_lines].p = malloc(length);
							memcpy(lines[data->num_lines].p, &buf[ptr], length);

							lines[data->num_lines].size = length;

							data->num_lines++;
						}
					}
					ptr = i+1;
				}
			}
		}
	}

	//Line-mix stage
	if(data->stage == 1){

		if(data->afl->queue_cur->was_fuzzed == 0){
			if(data->afl->stage_cur == 0)
				data->remaining++;
			return brute_8bits(data, buf, buf_size, out_buf, max_size);
		}

		if(data->afl->queue_cur->trim_done > 0){
			mutated_size = 0;
			data->afl->stage_max = 1;
			goto end;
		}

		u_int n_lines;

		if(data->afl->stage_cur == 0){

			n_lines = 1;
			for(int i=0; i<buf_size; i++){
				if(buf[i] == separator){
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

		while(ptr < buf_size && buf[ptr] != separator){
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
			data->afl->queue_cur->trim_done = 1;
			data->remaining--;
			data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
		}
	}

	end:
	*out_buf = mutated_out;
	return mutated_size;
}


ize_t afl_custom_fuzz(my_mutator_t *data, uint8_t *buf, size_t buf_size,
                       u8 **out_buf, uint8_t *add_buf, size_t add_buf_size,
                       size_t max_size) {


		size_t tmp = lines_mix(data, buf, buf_size, out_buf, max_size, ' ');
		return tmp;

}




