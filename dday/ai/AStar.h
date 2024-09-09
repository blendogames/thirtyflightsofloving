
//==========================================
// 
// 
//==========================================

typedef struct astarpath_s
{
	int numNodes;
	short int nodes[2048];//MAX_NODES jabot092(2)
	int originNode;
	int goalNode;

} astarpath_t;

//	A* PROPS
//===========================================
int	AStar_nodeIsInClosed( int node );
int	AStar_nodeIsInOpen( int node );
int	AStar_nodeIsInPath( int node );
static int	AStar_ResolvePath ( int origin, int goal, int movetypes );
//===========================================
int AStar_GetPath( int origin, int goal, int movetypes, struct astarpath_s *path );
