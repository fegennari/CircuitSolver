void set_valid_nodes(component *head);


void set_valid_nodes(component *head) {

	connections x = get_connections(head->location, head->id, head->flags);
	
	if (x.in1 != 0 && NODENUMBER[x.in1] == -2) 
		NODENUMBER[x.in1] = -1;
		
	if (x.in2 != 0 && NODENUMBER[x.in2] == -2) 
		NODENUMBER[x.in2] = -1;
		
	if (x.in3 != 0 && NODENUMBER[x.in3] == -2) 
		NODENUMBER[x.in3] = -1;
		
	if (x.out1 != 0 && NODENUMBER[x.out1] == -2) 
		NODENUMBER[x.out1] = -1;
		
	if (x.out2 != 0 && NODENUMBER[x.out2] == -2) 
		NODENUMBER[x.out2] = -1;
		
	if (x.out3 != 0 && NODENUMBER[x.out3] == -2) 
		NODENUMBER[x.out3] = -1;
}


else 
	set_valid_nodes(temp);