#pragma once

namespace srpg{

template<class Map, class Vertex, class Visitor>
void breadth_first_search(Map& map, Vertex& begin, Visitor&& visitor){
	// std::vector<bool> visit_list(map.get_data().size());
	std::vector<Vertex*> stack;
	stack.push_back(&begin);

	for(size_t i=0; i<=stack.size(); ++i){
		Vertex* p = stack[i];
		visitor.examine(map, *p, stack);
		visitor.process(map, *p);
	}
};

class status{
protected:
	Operation::list_type _history;

public:
	Operation::list_type& get_history(){ return _history; };

	void push();
	void apply(Operation::list_type&);
	void pop();
};


template<class AI>
Operation* think(status& s, AI&& ai){
	size_t unused;
	return calculateBestOperation(s, unused, std::forward<AI>(ai));
}

template<class AI>
Operation::list_type calculateBestOperation(status& s, size_t& maxScore, AI&& ai){
	Operation::list_type best;
	maxScore = 0;

	size_t score;
	std::vector<Operation::list_type> ops = ai.possible_moves(s);

	for(auto& op : ops){
		s.push();
		s.apply(op);

		// calculate score
		Operation::list_type bestOp = calculateBestOperation(s, score, std::forward<AI>(ai));
		score = ai.evaluate(s, bestOp, score);

		if(score > maxScore){
			maxScore = score;
			op.splice(op.end(), bestOp);
			best = std::move(op);
		}
		s.pop();
	}
	return best;
}



} // end namespace srpg
