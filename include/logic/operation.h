
template<class T>
struct list_hook{
	T* next;
	T* prev;
};

namespace srpg{

class Operation{
public:

protected:
	list_hook<Operation> _hook;
	size_t _type;

	Operation() : _type(0);
};

} // end namespace srpg
