#include "CircuitSolver.h"
#include "Netlist.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.PlaceAndRoute.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Place and Routing for Netlist Import
// By Frank Gennari
extern int skip_placement_check, has_ground_net, has_vdd_net, cScale;
extern int x_blocks, y_blocks;


component *autoplace_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets);
component *place_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets, int quality);
component *route_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets, int quality);
component *route_wire(component *head, int *net_point, int pin1, int pin2);
component *reroute_diag_wire(component *head, int w_start, int &comp_id);
box get_bbox(int id, char flags);
component *comp_at_loc(component *head, component *&last, int loc, int height, int width, int wire_only, int pins_count);
int is_loc_on_comp(component *comp, int loc, int height, int width, int pins_count);

int get_ic_pins(int comp_id);




component *autoplace_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets) {
	
	int quality(0);

	cout << "\nCreating Circuit..." << endl;

	if (num_components == 0) {
		skip_placement_check = 0;
		return head;
	}
	/*cout << "Use better quality but slower place and route?  ";
	if (decision()) {
		cout << "Sorry, this option is not yet supported." << endl; // *** CHANGE ***
		quality = 1;
	}*/
	reset_timer(1);
	show_elapsed_time();
	cout << "Placing Components..." << endl;
	head = place_components(head, ic_net_list, cnets, num_components, num_nets, quality);
	
	show_elapsed_time();
	cout << "Routing Circuit..." << endl;
	head = route_components(head, ic_net_list, cnets, num_components, num_nets, quality);
	
	show_elapsed_time();
	cout << "Done." << endl;
	skip_placement_check = 0;
	
	return head;
}




component *place_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets, int quality) {

	unsigned int i;
	box bbox;
	component *temp_comp = head;
	comp_nets *temp_cnets = cnets;
	ICNet_list *temp_icnets = ic_net_list;
	int *net_center = NULL, *num_net_conns = NULL, conns_array[6];

	if (num_components == 0 || num_nets == 0)
		return head;

	if (quality > 0) {
		;// WRITE
	}
	net_center    = memAlloc_init(net_center,    num_nets, 0);
	num_net_conns = memAlloc_init(num_net_conns, num_nets, 0);

	while (temp_comp != 0) {
		if (get_order(temp_comp->id) == icx) {
			// do something?
			temp_icnets = temp_icnets->next;
		}
		else {
			bbox = get_bbox(temp_comp->id, temp_comp->flags);
			
			conns_array[0] = temp_cnets->nets.in1;
			conns_array[1] = temp_cnets->nets.in2;
			conns_array[2] = temp_cnets->nets.in3;
			conns_array[3] = temp_cnets->nets.out1;
			conns_array[4] = temp_cnets->nets.out2;
			conns_array[5] = temp_cnets->nets.out3;
			
			//cout << "\nConns: ";
			
			for (i = 0; i < 6; ++i) {
				if (conns_array[i] >= num_nets) {
					internal_error();
					cerr << "Error in place_components: Net number " << conns_array[i] << " is >= the number of nets " << num_nets << "." << endl;
				}
				else if (conns_array[i] >= 0) {
					//net_center[conns_array[i]] += position/num_net_conns[conns_array[i]];
					++num_net_conns[conns_array[i]];
					//cout << conns_array[i] << " ";
				}
			}
			
			
			temp_cnets = temp_cnets->next;
		}
		temp_comp = temp_comp->next;
	}
	delete [] net_center;
	delete [] num_net_conns;	
		
	return head;
}




component *route_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets, int quality) {

	box bbox;
	connections conn, nets;
	component *temp_comp = head;
	comp_nets *temp_cnets = cnets;
	ICNet_list *temp_icnets = ic_net_list;
	int *net_point = NULL;
	
	if (num_components == 0 || num_nets == 0)
		return head;
	
	if (quality > 0) {
		;// WRITE
	}
	net_point = memAlloc_init(net_point, num_nets, -1);

	if (has_ground_net)	
		net_point[0] = GROUND_LOC;
	if (has_vdd_net)	
		net_point[has_vdd_net] = GROUND_LOC+1;
	
	while (temp_comp != 0) {
		if (get_order(temp_comp->id) == icx) {
			// handle ICs
			temp_icnets = temp_icnets->next;
		}
		else {
			conn = get_connections(temp_comp->location, temp_comp->id, temp_comp->flags);
			bbox = get_bbox(temp_comp->id, temp_comp->flags); // use
			nets = temp_cnets->nets;
			head = route_wire(head, net_point, conn.in1,  nets.in1);
			head = route_wire(head, net_point, conn.in2,  nets.in2);
			head = route_wire(head, net_point, conn.in3,  nets.in3);
			head = route_wire(head, net_point, conn.out1, nets.out1);
			head = route_wire(head, net_point, conn.out2, nets.out2);
			head = route_wire(head, net_point, conn.out3, nets.out3);
			temp_cnets = temp_cnets->next;
		}
		temp_comp = temp_comp->next;
	}
	if (has_ground_net)
		head = update_circuit(GROUND_ID, GROUND_LOC,   head, 0, 0);
	if (has_vdd_net)
		head = update_circuit(POWER_ID,  GROUND_LOC+1, head, 0, 0);
		
	delete [] net_point;

	return head;
}




component *route_wire(component *head, int *net_point, int pin1, int pin2) {

	if (pin1 > 0 && pin2 >= 0) {
		if (net_point[pin2] == -1)
			net_point[pin2] = pin1;
		else {
			head = update_circuit((Wire_base_id + net_point[pin2]), pin1, head, 2, 0);
		}
	}
	return head;
}



// MAKE BETTER!
component *reroute_diag_wire(component *head, int w_start, int &comp_id) {

	int x_dist, y_dist, corner1, corner2, w_end = comp_id - Wire_base_id;
	component *last; // dummy variable

	cout << "Diagonal wire." << endl; // testing
		
	if (w_start > w_end)
		swap(w_start, w_end);
		
	x_dist  = w_end%x_blocks - w_start%x_blocks;
	y_dist  = w_end/x_blocks - w_start/x_blocks;
	corner1 = w_start + y_dist*x_blocks;
	corner2 = w_start + x_dist;
	
	// choose corner
	if (comp_at_loc(head, last, corner1, 0, 0, 0, 1) == NULL) { // corner1 good
		head = update_circuit(comp_id, corner1, head, 2, 0);
		comp_id = corner1 + Wire_base_id;
	}
	else if (comp_at_loc(head, last, corner2, 0, 0, 0, 1) == NULL) { // corner2 good
		head = update_circuit(comp_id, corner2, head, 2, 0);
		comp_id = corner2 + Wire_base_id;
	}
	else { // corners are bad, leave diagonal for now
		cout << "Can't reroute." << endl;
	}	
	return head;
}




box get_bbox(int id, char flags) {

	box bbox;
	int order = get_order(id), base_id(-1);
	
	switch (order) {	
		case wirex:
			bbox.h_boxes = 0;
			bbox.v_boxes = 0;
			break;
			
		case icx:
			bbox.h_boxes = get_ic_pins(id)/2;
			bbox.v_boxes = comp_widths[IC_ID];
			break;
			
		case transistorx:
			base_id = TRANSISTOR_ID;
			
		case flipflopx:
			if (base_id == -1)
				base_id = FF_ID;
			
		default:
			if (base_id == -1)
				base_id = id;
				
			if (base_id <= MAX_USED_ID) {
				bbox.h_boxes = comp_lengths[base_id];
				bbox.v_boxes = comp_widths[base_id];
				if ((flags & ROTATED) && comp_rotation[base_id] == 1) { // rotated 90 degrees
					swap(bbox.h_boxes, bbox.v_boxes);
				}
			}
			else {
				bbox.h_boxes = 0;
				bbox.v_boxes = 0;
			}
	}
	bbox.h_boxes *= cScale;
	bbox.v_boxes *= cScale;
	
	return bbox;
}




component *comp_at_loc(component *head, component *&last, int loc, int height, int width, int wire_only, int pins_count) {

	last = NULL;

	while (head != 0) {
		if ((!wire_only || get_order(head->id) == wirex) && is_loc_on_comp(head, loc, height, width, pins_count)) {
			return head;
		}
		last = head;
		head = head->next;
	}
	return NULL;
}




int is_loc_on_comp(component *comp, int loc, int height, int width, int pins_count) {

	int comp_x, comp_y, comp_x1, comp_y1, comp_x2, comp_y2, loc_x, loc_y;
	box bbox;
	
	if (comp == NULL)
		return 0;
	
	loc_x = loc%x_blocks;
	loc_y = loc/x_blocks;
	
	if (get_order(comp->id) == wirex) {
		comp_x1 = comp->location%x_blocks;
		comp_y1 = comp->location/x_blocks;
		comp_x2 = (comp->id - Wire_base_id)%x_blocks;
		comp_y2 = (comp->id - Wire_base_id)/x_blocks;
		
		if (comp_x1 > comp_x2)
			swap(comp_x1, comp_x2);
		if (comp_y1 > comp_y2)
			swap(comp_y1, comp_y2);
				
		if (comp_x1 == comp_x2) {
			if (!pins_count) { // vertical
				++comp_y1;
				--comp_y2;
			}			
		}
		else if (comp_y1 == comp_y2) {
			if (!pins_count) { // horizontal
				++comp_x1;
				--comp_x2;
			}
		}
		else
			return 0; // diagonal, not dealt with
	}
	else {	
		bbox    = get_bbox(comp->id, comp->flags);
		comp_x  = comp->location%x_blocks;
		comp_y  = comp->location/x_blocks;
		comp_x1 = comp_x - bbox.h_boxes/2;
		comp_y1 = comp_y - bbox.v_boxes/2;
		comp_x2 = comp_x + bbox.h_boxes/2;
		comp_y2 = comp_y + bbox.v_boxes/2;
		
		if (!pins_count) {
			if (get_order(comp->id) == icx) {
				++comp_y1;
				--comp_y2;
			}
			else {
				++comp_x1;
				--comp_x2;
			}
		}
	}
	//return (loc_x >= comp_x1 && loc_x <= comp_x2 && loc_y >= comp_y1 && loc_y <= comp_y2);
	return !(loc_x > comp_x2 || (loc_x + width) < comp_x1 || loc_y > comp_y2 || (loc_y + height) < comp_y1);
}









