/*PGR-GNU*****************************************************************
File: vrp_vroom.hpp

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438@gmail.com

------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
********************************************************************PGR-GNU*/

#ifndef INCLUDE_VROOM_VRP_VROOM_HPP_
#define INCLUDE_VROOM_VRP_VROOM_HPP_
#pragma once


// #include <visitors/dfs_visitor.hpp>
// #include <boost/graph/depth_first_search.hpp>
// #include <boost/graph/undirected_dfs.hpp>

#include <vector>
#include <map>
#include <string.h>

// #include "cpp_common/pgr_base_graph.hpp"
// #include "cpp_common/interruption.h"
#include "cpp_common/pgr_messages.h"


/** @file vrp_vroom.hpp
 * @brief The main file which calls the vroom instance.
 *
 * Contains actual implementation of the function, makes connection
 * to the running instance of vroom, and passes the parameters. Finally,
 * it does error handling and returns the json solution as a string.
 */


namespace pgrouting {
namespace functions {

//*************************************************************

class Vrp_vroom : public vrprouting::Pgr_messages {
 public:

     /** @name Vroom
      * @{
      *
      */

     /** @brief vroom function
      *
      * It does all the processing and returns the results.
      *
      * @param graph      the graph containing the edges
      * @param roots      the root vertices
      * @param max_depth  the maximum depth of traversal
      * @param directed   whether the graph is directed or undirected
      *
      * @returns results, when results are found
      */
     std::vector < vrp_vroom_rt > vroom(
             std::string problem_instance_json,
             std::string server_host,
             std::string server_port,
             bool plan,
             bool geometry) {
         std::vector < vrp_vroom_rt > results;
         std::string solution_str = "Hi there!!!";
         log << "Hoho\n";
         char *solution = new char[solution_str.length() + 1];
         // const char *solution = solution_str.c_str();
         strcpy(solution, solution_str.c_str());
         log << solution << '\n';
         // memcpy(solution, solution_str.c_str(), solution_str.size() + 1);
         results.push_back({solution});
         for (auto i : results) {
          log << i.solution << " ok\n";
         }

#if 0
         for (auto root : roots) {
             std::vector < E > visited_order;
             results.push_back({root, 0, root, -1, 0.0, 0.0});

             if (graph.has_vertex(root)) {
                 auto v_root(graph.get_V(root));

                 depthFirstSearch_single_vertex(graph, v_root, visited_order, directed, max_depth);

                 auto result = get_results(visited_order, root, max_depth, graph);
                 results.insert(results.end(), result.begin(), result.end());
             }
         }
#endif

         return results;
     }

     //@}
#if 0
 private:
     /** @brief Calls the Boost function
      *
      * Calls [boost::depth_first_search]
      * (https://www.boost.org/libs/graph/doc/depth_first_search.html)
      * and [boost::undirected_dfs]
      * (https://www.boost.org/libs/graph/doc/undirected_dfs.html)
      * for directed graphs and undirected graphs, respectively.
      *
      * @param graph          the graph containing the edges
      * @param root           the root vertex
      * @param visited_order  vector which will contain the edges of the resulting traversal
      * @param directed       whether the graph is directed or undirected
      *
      * @returns bool  @b true, when results are found
      */
     bool depthFirstSearch_single_vertex(
                 G &graph,
                 V root,
                 std::vector < E > &visited_order,
                 bool directed,
                 int64_t max_depth) {
         using dfs_visitor = visitors::Dfs_visitor < V, E, G >;

         // Exterior property storage containers
         std::vector < boost::default_color_type > colors(boost::num_vertices(graph.graph));
         std::map < E, boost::default_color_type > edge_color;

         auto i_map = boost::get(boost::vertex_index, graph.graph);

         // Iterator property maps which record the color of each vertex and edge, respectively
         auto vertex_color_map = boost::make_iterator_property_map(colors.begin(), i_map);
         auto edge_color_map = boost::make_assoc_property_map(edge_color);

         auto vis =  dfs_visitor(root, visited_order, max_depth, colors, graph);

         /* abort in case of an interruption occurs (e.g. the query is being cancelled) */
         CHECK_FOR_INTERRUPTS();

         try {
             if (directed) {
                 boost::depth_first_search(graph.graph, vis, vertex_color_map, root);
             } else {
                 boost::undirected_dfs(graph.graph, vis, vertex_color_map, edge_color_map, root);
             }
         } catch(found_goals &) {
             {}
         } catch (boost::exception const& ex) {
             (void)ex;
             throw;
         } catch (std::exception &e) {
             (void)e;
             throw;
         } catch (...) {
             throw;
         }
         return true;
     }

     /** @brief to get the results
      *
      * Uses the `visited_order` of vertices to get the results.
      * Selects only those nodes in the final result whose
      * depth is less than the `max_depth`.
      *
      * @param visited_order  vector which contains the edges of the resulting traversal
      * @param root           the root vertex
      * @param max_depth      the maximum depth of traversal
      * @param graph          the graph containing the edges
      *
      * @returns `results` vector
      */
     template < typename T >
     std::vector < pgr_mst_rt > get_results(
             T visited_order,
             int64_t root,
             int64_t max_depth,
             const G &graph) {
         std::vector < pgr_mst_rt > results;

         std::vector < double > agg_cost(graph.num_vertices(), 0);
         std::vector < int64_t > depth(graph.num_vertices(), 0);

         for (const auto edge : visited_order) {
             auto u = graph.source(edge);
             auto v = graph.target(edge);

             agg_cost[v] = agg_cost[u] + graph[edge].cost;
             depth[v] = depth[u] + 1;

             if (max_depth >= depth[v]) {
                 results.push_back({
                     root,
                     depth[v],
                     graph[v].id,
                     graph[edge].id,
                     graph[edge].cost,
                     agg_cost[v]
                 });
             }
         }
         return results;
     }
#endif
};
}  // namespace functions
}  // namespace pgrouting

#endif  // INCLUDE_VROOM_VRP_VROOM_HPP_
