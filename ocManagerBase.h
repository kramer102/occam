/* Copyright 2000, Portland State University Systems Science Program.  All Rights Reserved
 */
#ifndef OCMANAGER_BASE_H
#define OCMANAGER_BASE_H

#include "ocOptions.h"

/**
 * ocIntersectProcessor - this is a base class for processing classes
 * used by ocManagerBase::doIntersectProcessing. This function does the
 * alternating relation intersect traversal for a model, used for computing
 * DF, H, and other functions.
 * To use this class, create an appropriate subclass, initialize it
 * properly, and pass it to doIntersectProcessing. Your process() function
 * will be called with all the relations of the model, then their first
 * order intersections, etc. For example, for model ABC:BCD:DEF, it will
 * be called with these relations/signs: ABC+, BCD+, DEF+, BC-, D-, NULL-
 * (the Null occurs because there is no intersection between ABC and DEF)
 */

class ocIntersectProcessor
{
public:
	ocIntersectProcessor() {}

	//-- this function is called for each relation. sign is +1 for
	//-- the first, third, etc. levels, and -1 for second, fourth, etc.
	virtual void process(int sign, ocRelation *rel) = 0;
};

/**
 * ocManagerBase - implements base functionality of an ocManager.  This class is the
 * provider for algorithms which manipulate core objects.  The class is extensible,
 * to allow new managers to be developed for different analysis approaches.
 *
 * The base manager has default implementions for commonly required operations, such as:
 * - generation of descendent models from an existing model
 * - creation of a table for a model, by projection from an input table
 * - creation of a fit table for a model using IPF
 * - calculation of various parameters for a table or model
 * - determination if a model contains loops
 */
 
 class ocManagerBase {
 public:
 	// method to use for computing H. Auto means use IPF if there are loops and
	// algebraic method otherwise.
	enum HMethod {AUTO, IPF, ALGEBRAIC};
	
 	// create an ocManagerBase object, supplying it with a variable list and a table
	// of input data.  Typically an application will read the input data and variable
	// definitions, and then construct the appropriate manager.
	ocManagerBase(ocVariableList *vars = 0, ocTable *input = 0);
	
	// initialize an ocManagerBase object, reading in standard options and data files.
	bool initFromCommandLine(int argc, char **argv);
	
	// delete this object
	virtual ~ocManagerBase();
	
	// get the relation defined by the list of variable indices and the count.
	// this will first search the cache and, if the relation is found, return it.
	// if not in the cache, a new relation is created. If instructed, the projection
	// corresponding to the relation is also created.
	virtual ocRelation *getRelation(int *varindices, int varcount, bool makeProject = false);

	// get a relation just like the given relation, but with the given variable removed.
	// (skip is the position within the relation, not the variable id).
	virtual ocRelation *getChildRelation(ocRelation *rel, int skip, bool makeProject = false);
	
 	// make a projection.  This creates an ocTable, and installs it as the table for the
	// given relation.  The projection is determined from the given table, and the
	// VariableList is the one associated with the given relation. False is returned
	// on any error. This function returns true immediately if the relation already
	// has a table.
 	virtual bool makeProjection(ocRelation *rel);

	// make a projection of table t1 into t2 (empty), based on the variable
	// list contained in the given relation. This is used as one step of
	// the IPF algorithm.
	virtual bool makeProjection(ocTable *t1, ocTable *t2, ocRelation *rel);
	
	// make a "maxProjection" of one table into another. This creates a partial
	// probability distribution by keeping only the max values for each matching tuple.
	// rel provides the set of variables to be used in the match (generally the IV set).
	virtual bool makeMaxProjection(ocTable *t1, ocTable *t2, ocTable *inputData, ocRelation *rel);
		
	// make projections for all relations in a model. This just calls makeProject
	// as many times as needed.
	virtual bool makeProjections(ocModel *model);

	// delete projection tables from all relations in cache
	virtual void deleteTablesFromCache();
	
	// make a fit table. This function uses the IPF algorithm. The fit table is
	// linked to the model.  If the model already has a fit table, the function
	// returns immediately. False is returned on any error
	virtual bool makeFitTable(ocModel *model);
	
	// process relations and intersections, as need for DF and H computation
	void doIntersectionProcessing(ocModel *model, ocIntersectProcessor *proc);

	// Compute degrees of freedom of a model.  This involves computing degrees
	// of freedom of the constituent relations, minus the first order overlaps,
	// plus the second order overlaps,etc. This also computes entropy, though
	// it isn't correct if the model contains loops. For models with loops, use
	// IPF.
	void DFAndEntropy(ocModel *model);
	
	// Determine if the model has loops, and cache this fact for later use
	bool hasLoops(ocModel *model);
	
	// compute various relation and model attributes. False is returned on error
	// (e.g., no table has been computed) If computation is successful, the
	// attribute is added to the attribute list for the relation or model.
	virtual double computeDF(ocRelation *rel);	// degrees of freedom
	virtual double computeDF(ocModel *model);
	virtual double computeH(ocRelation *rel);	// uncertainty
	virtual double computeH(ocModel *model, HMethod method = AUTO);
	virtual double computeTransmission(ocModel *model, HMethod method = AUTO);
	virtual void computeStatistics(ocRelation *rel);
	virtual void computeRelWidth(ocModel *model);
	
	int getKeySize() { return keysize; }
	double getSampleSz(){return (double)sampleSize;}
	ocVariableList *getVariableList() { return varList;}
	class ocRelCache *getRelCache() { return relCache; }
	class ocModelCache *getModelCache() { return modelCache; }
	class ocTable *getInputData() { return inputData; }
	class ocTable *getTestData() { return testData; }

	//-- generate a model, given the name. This assumes "." as variable separator and
	//-- ":" as relation separator. Optionally the data projection can be created.
	ocModel *makeModel(const char *name, bool makeProject = true);
	
	//-- get and set options. The options are initialized by command line or datafile options
	bool setOptionString(ocOptionDef *def, const char *value)
		{ return options->setOptionString(def, value); }
	bool setOptionFloat(ocOptionDef *def, double nvalue)
		{ return options->setOptionFloat(def, nvalue); }
	bool getOptionString(const char *name, void **next, const char **value)
		{ return options->getOptionString(name, next, value); }
	bool getOptionFloat(const char *name, void **next, double *nvalue)
		{ return options->getOptionFloat(name, next, nvalue); }

	void printOptions(bool printHTML);
	
	class ocTable *getFitTable() { return fitTable1; }

	//-- Print debug info on memory usage
	void printSizes();


protected:
	ocVariableList *varList;
	int keysize;
	int sampleSize;
	long stateSpaceSize;
	ocTable *inputData;
	ocTable *testData;
	double inputH;
	class ocRelCache *relCache;
	class ocModelCache *modelCache;
	class ocOptions *options;
 	ocTable *fitTable1;
	ocTable *fitTable2;
	ocTable *projTable;
};
 
 #endif
 

