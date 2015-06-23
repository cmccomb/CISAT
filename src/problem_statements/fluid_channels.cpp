#include "../../include/problem_statements/fluid_channels.hpp"


// Graph grammar characteristics
const  unsigned long  Solution::number_of_move_ops   = 4;
const  unsigned long  Solution::number_of_objectives = 1;
const  std::string    Solution::name                 = "Gravity Fed Fluid Network";
const  long double    Solution::goal                 = 0.0;

//TODO: Add a vector that contains pipe sizes

// Fluid constants
const  long double    Solution::fluid_n              = 1.85;
const  long double    Solution::fluid_C              = 100;

enum NodeTypes {INLET=1, INTERMEDIATE, OUTLET};

// Problem definition
std::vector< std::map<std::string, long double> > Solution::seed_graph_parameters = {
        {
                {"x",  0.00},
                {"y",  0.00},
                {"z", 25.00},
                {"type", INLET}
        }, {
                {"x", 25.00},
                {"y",  0.00},
                {"z",  0.00},
                {"q",  0.00},
                {"type", OUTLET}
        }, {
                {"x", 25.00},
                {"y", 25.00},
                {"z",  0.00},
                {"q",  0.00},
                {"type", OUTLET}
        }, {
                {"x",  0.00},
                {"y", 25.00},
                {"z",  0.00},
                {"q",  0.00},
                {"type", OUTLET}
        }
};


// Integer to assign unique IDs to solutions
int Solution::solution_counter = 0;


// Null constructor
Solution::Solution(void) {}


// A real constructor
Solution::Solution(bool) {
    // Give the solution a unique ID and increment the counter
    solution_id = solution_counter;
    solution_counter++;

    // Create seed graph
    create_seed_graph();

    // Initialize quality
    quality.assign(number_of_objectives, LDBL_MAX);
    compute_quality();
}


void Solution::create_seed_graph(void) {
    // Create nodes at each of the specified locations
    for(int i=0; i < seed_graph_parameters.size(); i++) {
        add_junction(seed_graph_parameters[i]["x"],
                     seed_graph_parameters[i]["y"],
                     seed_graph_parameters[i]["z"],
                     false);
        nodes[node_id_counter].parameters["type"] = seed_graph_parameters[i]["type"];
        if(seed_graph_parameters[i]["type"] == INLET) {
            inlet_keys.push_back(node_id_counter);
        } else {
            outlet_keys.push_back(node_id_counter);
        }
    }
}


void Solution::compute_quality(void) {
    // For now, just sum the length
    quality[0] = 1000;
    int validity = is_valid();
    for(std::map<int, Edge>::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
        quality[0] -= edges[iter->first].parameters["L"];
    }
    quality[0] += 0.2*number_of_nodes;
    quality[0] += 10*validity;

    // TODO: Real quality computation
    // update_geometry();
    // solve_flow();
    // for(int i=0; i<outlet_keys.size(); i++) {
    //     quality[0] = std::pow(0.05 - nodes[i].parameters["q"], 2);
    // }
    // for(std::map<int, Edge>::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
    //     quality[1] += edges[iter->first].parameters["m"];
    // }
}


void Solution::get_valid_moves(void) {
    std::vector<int> order;

    // Clear the options vector, and then instantiate it
    move_options.clear();
    move_options.assign(4, std::vector< std::vector<int> > ());

    // Pull valid edge addition moves and node deletion moves
    for (std::map<int, Node>::iterator it1 = nodes.begin(); it1 != nodes.end(); it1++) {
        if(nodes[it1->first].parameters["moveable"] == true){
            // Define the move
            order = {3, it1->first, 0};

            // Add it to the vector of options
            move_options[3].push_back(order);
        }
        for (std::map<int, Node>::iterator it2 = nodes.begin(); it2 != nodes.end(); it2++) {
            if (!undirected_edge_exists((it1->first), (it2->first))) {
                if((it2->first) != (it1->first)) {
                    // Define the move
                    order = {0, (it1->first), (it2->first)};

                    // Add it to the vector of options
                    move_options[0].push_back(order);
                }
            }
        }
    }

    // Pull valid junction addition and edge deletion moves
    for (std::map<int, Edge>::iterator it1 = edges.begin(); it1 != edges.end(); it1++) {
        // Define the junction addition move and add it
        order = {1, it1->first, 0};
        move_options[1].push_back(order);

        // Define the edge deletion move and add it
        order = {2, it1->first, 0};
        move_options[2].push_back(order);
    }
}


void Solution::apply_move_operator(int move_type, int move_number) {
    std::vector<int> selected_order = move_options[move_type][move_number];


    switch(selected_order[0]) {
        case 0:
            add_pipe(selected_order[1], selected_order[2], 0.1);
            break;
        case 1:
            add_midpoint_junction(selected_order[1]);
            break;
        case 2:
            remove_pipe(selected_order[1]);
            break;
        case 3:
            remove_junction(selected_order[1]);
            break;
        default:
            break;
    }

    // Compute the quality
    compute_quality();
    std::cout << std::endl << selected_order[0] << std::endl;
}


void Solution::add_pipe(int n1, int n2, long double d) {
    // Add an edge
    add_edge(n1, n2);

    // Add parameters for the edges
    edges[edge_id_counter].parameters["D"] = d;

    // Compute the length
    edges[edge_id_counter].parameters["L"] = euclidean_distance(n1, n2);
    edges[edge_id_counter].parameters["Q"] = 0.0;
    edges[edge_id_counter].parameters["k"] = 10.67/(std::pow(fluid_C, fluid_n) * std::pow(d, 4.87));
}


void Solution::add_junction(long double x, long double y, long double z, bool moveable) {
    // Add the node
    add_node();

    // Add the coordinates
    nodes[node_id_counter].parameters["x"] = x;
    nodes[node_id_counter].parameters["y"] = y;
    nodes[node_id_counter].parameters["z"] = z;

    // Moveable or not
    nodes[node_id_counter].parameters["moveable"] = moveable;
    nodes[node_id_counter].parameters["type"] = INTERMEDIATE;
}


void Solution::add_midpoint_junction(int e) {
    // Save the indices of the endpoints
    int n1 = edges[e].initial_node;
    int n2 = edges[e].terminal_node;

    // Remove the edge
    remove_pipe(e);

    // Add a new node
    add_junction(
            (nodes[n1].parameters["x"] + nodes[n2].parameters["x"])/2,
            (nodes[n1].parameters["y"] + nodes[n2].parameters["y"])/2,
            (nodes[n1].parameters["z"] + nodes[n2].parameters["z"])/2,
      false
    );

    // Add new edges
    add_pipe(n1, node_id_counter, 0.1);
    add_pipe(n2, node_id_counter, 0.1);
}


void Solution::remove_pipe(int e) {
    remove_edge(e);
}


void Solution::remove_junction(int n) {
    remove_node(n);
}


void Solution::print_surface_characteristics(void) {
    print("Problem Characteristics");
    std::cout << "\tName: " << name << std::endl;
    std::cout << "\tGoal: " << goal << std::endl;
    std::cout << "\tNumber of move operators: " << number_of_move_ops << std::endl;
}


// Function to ensure that the solution is valid
int Solution::is_valid(void) {
    return is_connected();
}


long double Solution::euclidean_distance(int n1, int n2) {
    return std::sqrt(
                  std::pow(nodes[n1].parameters["x"] - nodes[n2].parameters["x"], 2)
                + std::pow(nodes[n1].parameters["y"] - nodes[n2].parameters["y"], 2)
                + std::pow(nodes[n1].parameters["z"] - nodes[n2].parameters["z"], 2)
    );
}
