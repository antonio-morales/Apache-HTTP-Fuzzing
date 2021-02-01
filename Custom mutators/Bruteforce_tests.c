size_t brute_8bits(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;

	if(data->afl->queue_cur->was_fuzzed){
		mutated_size = 0;
		data->afl->stage_max = 1;
		goto end;
	}

	if(data->afl->stage_cur == 0){
		data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
		data->index_i = 0;
		data->byte_count_1 = 0;
	}

	//TODO: AVOID this multiplication every time
	data->afl->stage_max = buf_size*256;
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

	if(data->byte_count_1 == 256){

		if(data->brute_level > 1)
			add_dict_entry(mutated_out[data->index_i], mutated_out[data->index_i+1], mutated_out[data->index_i+2]);

		if(data->brute_level > 2)
			add_dict_entry_4B(mutated_out[data->index_i], mutated_out[data->index_i+1], mutated_out[data->index_i+2], mutated_out[data->index_i+3]);

		++data->index_i;
		data->byte_count_1 = 0;
	}

	mutated_out[data->index_i] = data->byte_count_1;

	++data->byte_count_1;


	if(data->afl->stage_cur >= buf_size*256-1){
		//printf("yeaahhh\n");
		data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
	}


	end:
	*out_buf = mutated_out;
	return mutated_size;
}


size_t brute_16bits(my_mutator_t *data, uint8_t *buf, size_t buf_size,
        u8 **out_buf, size_t max_size){

	size_t mutated_size;

	if(data->afl->queue_cur->was_fuzzed){
		mutated_size = 0;
		data->afl->stage_max = 1;
		goto end;
	}

	if(data->afl->stage_cur == 0){
		data->max_steps = (buf_size-1)*256*256 + buf_size;
		data->havoc_max_mult_OLD = data->afl->havoc_max_mult;
		data->index_i = 0;
		data->index_j = 1;
		data->byte_count_1 = 0;
		data->byte_count_2 = 0;
	}


	data->afl->stage_max = data->max_steps;
	data->afl->havoc_max_mult = 0;


	// Make sure that the packet size does not exceed the maximum size expected by the fuzzer
	mutated_size = buf_size <= max_size ? buf_size : max_size;

	// maybe_grow is optimized to be quick for reused buffers.
	u8 *mutated_out = maybe_grow((void**)&buf, &buf_size, mutated_size);
	if (!mutated_out) {

		*out_buf = NULL;
		perror("custom mutator allocation (maybe_grow)");
		return 0;
	}

	if(data->index_j == buf_size){
		++data->byte_count_1;
		data->index_j = data->index_i+1;
		data->byte_count_2 = 0;
	}

	if(data->byte_count_1 == 256){

		if(data->brute_level > 1)
			add_dict_entry(mutated_out[data->index_i], mutated_out[data->index_i+1], mutated_out[data->index_i+2]);

		++data->index_i;
		++data->index_j;
		data->byte_count_1 = 0;
		data->byte_count_2 = 0;
		if(data->index_j == buf_size)
			goto post;
	}

	if(data->byte_count_2 == 256){
		data->byte_count_1++;
		data->byte_count_2 = 0;
	}

	mutated_out[data->index_i] = data->byte_count_1;
	mutated_out[data->index_j] = data->byte_count_2;


	++data->byte_count_2;

	/*
	if(data->index_i==(buf_size-2))
		printf("yeeep\n");
	 */

	post:

	if(data->afl->stage_cur == data->afl->stage_max-1){
		//printf("yeaahhh\n");
		data->afl->havoc_max_mult = data->havoc_max_mult_OLD;
	}

	end:
	*out_buf = mutated_out;
	return mutated_size;
}