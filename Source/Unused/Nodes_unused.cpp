unsigned count_wires(component *head) {

	int ID;
	unsigned numwires(0);

	while (head != 0) {
		ID = head->id;

		if (get_order(ID) == wirex) {
			++numwires;
		}
		else if (!useCOMPLEX) {
			if (ID == INDUCTOR_ID) {
				++numwires;
			}
			else if (ID == XFMR_ID) {
				numwires += 2;
			}
		}
		head = head->next;
	}
	return numwires;
}




void create_wires(component *head, wire *wires) {

	int ID;
	unsigned index(0);
	connections x;

	while (head != 0) {
		ID = head->id;

		if (get_order(ID) == wirex) {
			wires[index].start = head->location;
			wires[index++].end = ID - Wire_base_id;
		}
		else if (!useCOMPLEX) {
			if (ID == INDUCTOR_ID) {
				x = get_connections(head->location, ID, head->flags);
				wires[index].start = x.in2;
				wires[index++].end = x.out2;
			}
			else if (ID == XFMR_ID) {
				x = get_connections(head->location, ID, head->flags);
				wires[index].start = x.in1;
				wires[index++].end = x.in3;
				wires[index].start = x.out1;
				wires[index++].end = x.out3;
			}
		}
		head = head->next;
	}
}
