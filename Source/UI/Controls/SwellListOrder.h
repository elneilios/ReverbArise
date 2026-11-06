#pragma once
#include <juce_core/juce_core.h>
#include <algorithm>
#include <numeric>
#include <span>
#include <vector>

namespace m3::ui::controls
{
// Indices belong to the supplied immutable snapshot, never to the session model.
// Direct comparisons also support the entire signed sample-coordinate domain.
template<class Row>
std::vector<size_t> chronologicalSwellOrder(std::span<const Row> rows)
{
    std::vector<size_t> order(rows.size());
    std::iota(order.begin(),order.end(),size_t{});
    std::sort(order.begin(),order.end(),[&](size_t a,size_t b)
    {
        if(rows[a].arrival!=rows[b].arrival)return rows[a].arrival<rows[b].arrival;
        return rows[a].id.compare(rows[b].id)<0;
    });
    return order;
}
}
