/* Authors:
 * Igor Rafael [igor@dcc.ufmg.br]
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */

#ifndef jsion_cp_h__
#define jsion_cp_h__

#include <set>
#include <map>
#include <list>
// This file represents the Constant Propagation optimization pass

namespace js {
namespace ion {

#ifdef DEBUG
#define OPEN_STATS_FILE(name) FILE * name = fopen("/tmp/cpStats","a");
#define SAVE_ACTUAL_SCRIPT(obj,script) obj.actualScript = script;
#define INIT_STATS_FILE { OPEN_STATS_FILE(_tmp)                                   \
					if(_tmp){                                                     \
						fprintf(_tmp, "Functions,Propagations,Instructions\n");   \
						fclose(_tmp);                                             \
					}                                                             \
					}
#define SAVE_STATS(s,n,t) { OPEN_STATS_FILE(_tmp)                                     \
					if(_tmp){                                                         \
						fprintf(_tmp, "%s:%d,%d,%d\n", s->filename, s->lineno, n, t); \
						fclose(_tmp);                                                 \
					}                                                                 \
					}
#define ANALYSE_STATE(s,b,c,t)  switch (s) {      \
					case Bottom:    b++;    break; \
					case Constant:  c++;    break; \
					case Top:       t++;    break; \
					}
#else
#define INIT_STATS_FILE
#define SAVE_STATS(s,n,t)
#define SAVE_ACTUAL_SCRIPT(obj,script)
#define ANALYSE_STATE(s,b,c,t)
#endif

class MIRGraph;
class MBasicBlock;
class MConstant;
class MPhi;

namespace cp {

/**
 * Phi cycle solving
 */
enum State 
{
    Bottom, 
    Constant, 
    Top
};

struct PhiItem
{
    State s;
    MConstant *c;

    PhiItem() { this->s = Bottom; this->c = NULL; }
    PhiItem(State s, MConstant* c){ this->s = s; this->c = c; }
};

} // namespace cp

/**
  * Constant propagation main class
  */
class CP
{
  public:
    CP(MIRGraph &graph);
    virtual ~CP() {}
    bool analyze();

#ifdef DEBUG
    JSScript * actualScript;
#endif

  protected:
    MIRGraph &graph;
    std::set<MDefinition*> workList;
    std::map<MPhi*, cp::PhiItem> phiMap;

    virtual bool initWorkList();
    bool isBlacklisted(MInstruction *ins);
    void addUsesToWorkList(MDefinition *def);
    void addPhiUsesToMap(MPhi *phi);
    bool analyzePhi(MPhi *phi, cp::PhiItem &pi);
    void solvePhiCycles();
    bool propagate(cp::PhiItem &to, cp::PhiItem &from);
#ifdef DEBUG
    int numFolded;
    int failsDueToType;
    int totalInst;
    inline void printStats();
    void inline countInst(){ totalInst++; }
    void inline countNumFolded(){ numFolded++; }
    void inline countFailsDueToType(){ failsDueToType++; }
#else
    inline void printStats(){}
    void inline countInst(){ }
	void inline countNumFolded(){ }
	void inline countFailsDueToType(){ }
#endif

};

} // namespace ion
} // namespace js

#endif // jsion_cp_h__
