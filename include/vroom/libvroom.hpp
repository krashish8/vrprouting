#include <iostream>

#include "routing/osrm_routed_wrapper.h"
#include "structures/vroom/input/input.h"
#include "structures/generic/matrix.h"
#include "structures/vroom/job.h"
#include "structures/vroom/vehicle.h"
#include "utils/exception.h"

// void log_solution(const vroom::Solution& sol, bool geometry) {
//   std::cout << "Total cost: " << sol.summary.cost << std::endl;
//   std::cout << "Unassigned: " << sol.summary.unassigned << std::endl;

//   // Log unassigned jobs if any.
//   std::cout << "Unassigned job ids: ";
//   for (const auto& j : sol.unassigned) {
//     std::cout << j.id << ", ";
//   }
//   std::cout << std::endl;

//   // Describe routes in solution.
//   for (const auto& route : sol.routes) {
//     std::cout << "Steps for vehicle " << route.vehicle
//               << " (cost: " << route.cost;
//     std::cout << " - duration: " << route.duration;
//     std::cout << " - service: " << route.service;
//     if (geometry) {
//       std::cout << " - distance: " << route.distance;
//     }

//     std::cout << ")" << std::endl;

//     // Describe all route steps.
//     for (const auto& step : route.steps) {
//       std::string type;
//       switch (step.step_type) {
//       case vroom::STEP_TYPE::START:
//         type = "Start";
//         break;
//       case vroom::STEP_TYPE::END:
//         type = "End";
//         break;
//       case vroom::STEP_TYPE::BREAK:
//         type = "Break";
//         break;
//       case vroom::STEP_TYPE::JOB:
//         switch (step.job_type) {
//         case vroom::JOB_TYPE::SINGLE:
//           type = "Job";
//           break;
//         case vroom::JOB_TYPE::PICKUP:
//           type = "Pickup";
//           break;
//         case vroom::JOB_TYPE::DELIVERY:
//           type = "Delivery";
//           break;
//         }
//         break;
//       }
//       std::cout << type;

//       // Add job/pickup/delivery/break ids.
//       if (step.step_type != vroom::STEP_TYPE::START and
//           step.step_type != vroom::STEP_TYPE::END) {
//         std::cout << " " << step.id;
//       }

//       // Add location if known.
//       if (step.location.has_coordinates()) {
//         std::cout << " - " << step.location.lon() << ";" << step.location.lat();
//       }

//       std::cout << " - arrival: " << step.arrival;
//       std::cout << " - duration: " << step.duration;
//       std::cout << " - service: " << step.service;

//       // Add extra step info if geometry is required.
//       if (geometry) {
//         std::cout << " - distance: " << step.distance;
//       }
//       std::cout << std::endl;
//     }
//   }
// }


void run_example_with_custom_matrix() {
  bool GEOMETRY = false;
  unsigned amount_dimension = 0; // No capacity constraint.

  vroom::Input problem_instance(amount_dimension);

  // Define custom matrix and bypass OSRM call.
  vroom::Matrix<vroom::Cost> matrix_input(4);

  matrix_input[0][0] = 0;
  matrix_input[0][1] = 2104;
  matrix_input[0][2] = 197;
  matrix_input[0][3] = 1299;
  matrix_input[1][0] = 2103;
  matrix_input[1][1] = 0;
  matrix_input[1][2] = 2255;
  matrix_input[1][3] = 3152;
  matrix_input[2][0] = 197;
  matrix_input[2][1] = 2256;
  matrix_input[2][2] = 0;
  matrix_input[2][3] = 1102;
  matrix_input[3][0] = 1299;
  matrix_input[3][1] = 3153;
  matrix_input[3][2] = 1102;
  matrix_input[3][3] = 0;

  problem_instance.set_matrix("car", std::move(matrix_input));

  // Define vehicles (use std::nullopt for no start or no end).
  vroom::Location v_start(0); // index in the provided matrix.
  vroom::Location v_end(3);   // index in the provided matrix.

  vroom::Vehicle v(0,       // id
                   v_start, // start
                   v_end);  // end
  problem_instance.add_vehicle(v);

  // Define jobs with id and index of location in the matrix
  // (coordinates are optional). Constraints that are not required can
  // be omitted.
  std::vector<vroom::Job> jobs;
  jobs.push_back(vroom::Job(1414, 1));
  jobs.push_back(vroom::Job(1515, 2));

  for (const auto& j : jobs) {
    problem_instance.add_job(j);
  }

  // Solve!
  auto sol = problem_instance.solve(5,  // Exploration level.
                                    4); // Use 4 threads.

  // log_solution(sol, GEOMETRY);
}
