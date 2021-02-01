size_t dict_3bytes(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;

	//trim_done = dict3 done
	if(data->afl->queue_cur->trim_done > 0){
		mutated_size = 0;
		data->afl->stage_max = 1;
		goto end;
	}

	if(data->afl->stage_cur == 0){
		data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
		data->index_i = 0;
	}

	data->afl->stage_max = data->dict3_size * (buf_size-2);
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

	if(data->index_i == 0){

		for(; data->i<256; data->i++){
			for(; data->j<256; data->j++){
				for(; data->k<256; data->k++){
					if(DICT3B[data->i][data->j][data->k]){
						goto found;
					}
				}
				data->k = 0;
			}
			data->j = 0;
		}
	}


	found:


	mutated_out[data->index_i] = data->i;
	mutated_out[data->index_i+1] = data->j;
	mutated_out[data->index_i+2] = data->k;
	data->index_i++;

	if(data->index_i == buf_size-2){
		data->index_i = 0;
		data->k++;
	}

	if(data->afl->stage_cur == data->afl->stage_max-1){
		//printf("yeaahhh\n");
		data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
		data->afl->queue_cur->trim_done = 1;
		data->i = 0; data->j = 0; data->k = 0;
		data->pending_dict3--;
	}


	end:
	*out_buf = mutated_out;
	return mutated_size;
}

size_t dict_4bytes(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;

	//trim_done = dict4 done
	if(data->afl->queue_cur->trim_done > 1){
		mutated_size = 0;
		data->afl->stage_max = 1;
		goto end;
	}

	if(data->afl->stage_cur == 0){
		data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
		data->index_i = 0;
	}

	data->afl->stage_max = data->dict4_size * (buf_size-3);
	data->afl->havoc_max_mult = 0;


	mutated_size = buf_size <= max_size ? buf_size : max_size;

	u8 *mutated_out = maybe_grow((void**)&buf, &buf_size, mutated_size);
	if (!mutated_out) {
		*out_buf = NULL;
		perror("custom mutator allocation (maybe_grow)");
		return 0;
	}

	mutated_out[data->index_i] = data->dict4[data->n];
	mutated_out[data->index_i+1] = data->dict4[data->n+1];
	mutated_out[data->index_i+2] = data->dict4[data->n+2];
	mutated_out[data->index_i+3] = data->dict4[data->n+3];
	data->index_i++;

	if(data->index_i == buf_size-3){
		data->index_i = 0;
		data->n += 4;
	}

	if(data->afl->stage_cur == data->afl->stage_max-1){
		data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
		data->afl->queue_cur->trim_done = 2;
		data->n = 0;
		data->pending_dict4--;
	}


	end:
	*out_buf = mutated_out;
	return mutated_size;
}