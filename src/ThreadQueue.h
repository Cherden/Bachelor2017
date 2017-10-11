#include <queue>
#include <mutex>

using namespace std;

class ThreadQueue{
public:
	int queueGet(){
		int i = 0;

		if (q.empty()){
			return -1;
		}
		
		m.lock();
		i = q.pop();
		m.unlock();

		return i;
	};

	void queueAdd(int val){
		m.lock();
		q.push(val);
		m.unlock();
	};
private:
	queue<int> q;
	mutex m;
};
