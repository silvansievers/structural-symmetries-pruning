#include "eager_search.h"
#include "search_common.h"

#include "../plugins/plugin.h"

#include "../structural_symmetries/group.h"

using namespace std;

namespace plugin_astar {
class AStarSearchFeature : public plugins::TypedFeature<SearchEngine, eager_search::EagerSearch> {
public:
    AStarSearchFeature() : TypedFeature("astar") {
        document_title("A* search (eager)");
        document_synopsis(
            "A* is a special case of eager best first search that uses g+h "
            "as f-function. "
            "We break ties using the evaluator. Closed nodes are re-opened.");

        add_option<shared_ptr<Evaluator>>("eval", "evaluator for h-value");
        add_option<shared_ptr<Evaluator>>(
            "lazy_evaluator",
                "An evaluator that re-evaluates a state before it is expanded.",
                plugins::ArgumentInfo::NO_DEFAULT);
        eager_search::add_options_to_feature(*this);

        document_note(
            "lazy_evaluator",
            "When a state s is taken out of the open list, the lazy evaluator h "
            "re-evaluates s. If h(s) changes (for example because h is path-dependent), "
            "s is not expanded, but instead reinserted into the open list. "
            "This option is currently only present for the A* algorithm.");
        document_note(
            "Equivalent statements using general eager search",
            "\n```\n--search astar(evaluator)\n```\n"
            "is equivalent to\n"
            "```\n--evaluator h=evaluator\n"
            "--search eager(tiebreaking([sum([g(), h]), h], unsafe_pruning=false),\n"
            "               reopen_closed=true, f_eval=sum([g(), h]))\n"
            "```\n", true);

        add_option<shared_ptr<Group>>(
            "symmetries",
            "symmetries object to compute structural symmetries for pruning",
            plugins::ArgumentInfo::NO_DEFAULT);
    }

    virtual shared_ptr<eager_search::EagerSearch> create_component(const plugins::Options &options, const utils::Context &context) const override {
        if (options.contains("symmetries")) {
            shared_ptr<Group> group = options.get<shared_ptr<Group>>("symmetries");
            if (group->get_search_symmetries() == SearchSymmetries::NONE) {
                context.error("Symmetries option passed to eager search, but no search symmetries should be used.");
            }
        }
        plugins::Options options_copy(options);
        auto temp = search_common::create_astar_open_list_factory_and_f_eval(options);
        options_copy.set("open", temp.first);
        options_copy.set("f_eval", temp.second);
        options_copy.set("reopen_closed", true);
        vector<shared_ptr<Evaluator>> preferred_list;
        options_copy.set("preferred", preferred_list);
        return make_shared<eager_search::EagerSearch>(options_copy);
    }
};

static plugins::FeaturePlugin<AStarSearchFeature> _plugin;
}
