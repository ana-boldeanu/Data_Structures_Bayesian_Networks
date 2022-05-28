#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING 10

// Structura pt un nod din lista de adiacenta
typedef struct TNode {
    int index;  // Indexul nodului in vectorul de liste de adiacenta
	char *name; // Valoarea din nod
	struct TNode *next; // Link catre urmatorul
} TNode;

// Structura pt graful orientat
typedef struct {
	int nrNodes; // Numarul de noduri din graf
	TNode **adjList; // Vectorul de liste de adiacenta
} TGraphL;

// Functie pt alocarea memoriei pt graf
TGraphL* createGraph(int nrNodes) {
	TGraphL *new_graph = (TGraphL *) calloc(1, sizeof(TGraphL));

	new_graph->nrNodes = nrNodes;
	new_graph->adjList = (TNode **) malloc(nrNodes * sizeof(TNode *));

	for (int i = 0; i < nrNodes; i++) {
		new_graph->adjList[i] = NULL;	// Initializez fiecare lista la NULL
	}

	return new_graph;
}

int contains(TNode *list, int index) {
	while (list != NULL) {
		if (list->index == index) {
			return 1;
		}
		list = list->next;
	}
	return 0;
}

// Functie simpla care adauga un nod intr-o lista (doar indexul nodului)
TNode *add_toList(TNode *list, int index) {
	if (contains(list, index)) {
		return list;
	}

	TNode *new_list = calloc(1, sizeof(TNode));

	new_list->index = index;
	new_list->next = list;

	return new_list;
}

// Determina indexul la care se afla name in vectorul de nume de noduri
int get_index(char **nodeNames, int nrNodes, char *name) {
    for (int i = 0; i < nrNodes; i++) {
        if (strcmp(nodeNames[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// Dezaloca memoria ocupata de graf si vectorul de nume de noduri
void killGraph(TGraphL *graph) {
	for (int i = 0; i < graph->nrNodes; i++) {
        // Memoria pt graf
		TNode *aux = graph->adjList[i];
		while (aux != NULL) {
			TNode *tmp = aux;
			aux = aux->next;
            free(tmp->name);
			free(tmp);
		}
	}
	free(graph->adjList);
	free(graph);
	return;
}

void killList(TNode *list) {
	TNode *aux = list;
	while (aux != NULL) {
		list = list->next;
		free(aux->name);
		free(aux);
		aux = list;
	}
	return;
}

// Foloseste acumulator ca sa construiasca pe parcurs o lista cu stramosii nodului childName
void get_ancestorsList(TGraphL *graph, char **nodeNames, char *childName, TNode **accList) {
    for (int i = 0; i < graph->nrNodes; i++) {
        TNode *aux = graph->adjList[i];
        while (aux != NULL) {
            if (strcmp(aux->name, childName) == 0) {
                int add = 1;
                TNode *tmp = *accList;
                while (tmp != NULL) {
                    if (tmp->index == i) { // Exista deja in lista de parinti
                        add = 0;
                        break;
                    }
                    tmp = tmp->next;
                }
                if (add) {  // Daca nu exista deja
                    TNode *parent = calloc(1, sizeof(TNode));
                    parent->index = i;
                    parent->next = *accList;
                    get_ancestorsList(graph, nodeNames, nodeNames[i], &parent);
                    *accList = parent;
                }
            }
            aux = aux->next;
        }
    }
}

// Intoarce lista cu indecsii parintilor. Prin argument, intoarce si numarul acestora
TNode* get_parentsList(TGraphL *graph, int childIdx, int *parents_nr) {
	TNode *parentsList = NULL;
	*parents_nr = 0;

    for (int i = 0; i < graph->nrNodes; i++) {
		// Trec prin fiecare lista de adiacenta si verific daca il are copil
        TNode *aux = graph->adjList[i];
        while (aux != NULL) {
            if (aux->index == childIdx) {
				parentsList = add_toList(parentsList, i);
				*parents_nr = *parents_nr + 1;
            }
            aux = aux->next;
        }
    }
	return parentsList;
}

// Functie care adauga nodul Y in lista de adiacenta a nodului X
void add_YtoXList(TGraphL *graph, char *Yname, int Xindex, int Yindex) {
	TNode *new_XList = (TNode *) calloc(1, sizeof(TNode));

	if (!contains(graph->adjList[Xindex], Yindex)) {
		new_XList->index = Yindex;
		new_XList->name = calloc(MAX_STRING, sizeof(char));
		strcpy(new_XList->name, Yname);

		new_XList->next = graph->adjList[Xindex];
		graph->adjList[Xindex] = new_XList;

	} else {
		killList(new_XList);
	}

	return;
}

// Functie care sterge un nod dintr-o lista (conteaza doar indexul sau)
TNode *remove_fromList(TNode *list, int index) {
	if (list == NULL) {
		return NULL;
	}

	if (contains(list, index)) {
		// Daca se afla pe prima pozitie
		if (list->index == index) {
			TNode *aux = list;
			list = list->next;
			free(aux->name);
			free(aux);
			return list;
		}

		// Daca nu
		TNode *prev = list;
		TNode *curr = list->next;
		while (curr != NULL) {
			if (curr->index == index) {
				prev->next = curr->next;
				free(curr->name);
				free(curr);
				return list;
			}
			prev = curr;
			curr = curr->next;
		}
	}
	return list;
}

// Functie care realizeaza un graf copie
TGraphL *copy_Graph(TGraphL *graph, char **nodeNames) {
	TGraphL *new_graph = createGraph(graph->nrNodes);

	for (int i = 0; i < graph->nrNodes; i++) {
		TNode *aux = graph->adjList[i];
		while (aux != NULL) {
			int Yindex = aux->index;
			int Xindex = i;

			add_YtoXList(new_graph, aux->name, Xindex, Yindex);

			aux = aux->next;
		}
	}

	return new_graph;
}

// DFS recursiv pe graf neorientat ca sa vad daca exista path intre X (initial) si Y
int DFS_areConnected(TGraphL *graph, int *visited, int init, int curr) {
	// Retin ca nodul a fost vizitat
	visited[curr] = 1;

	// Verific daca s-a ajuns la nodul initial
	if (init == curr) {
		return 1;
	}

	int path = 0;

	// Apelez recursiv pentru toti vecinii nevizitati
	TNode *aux = graph->adjList[curr];

	while (aux != NULL) {
		if (!visited[aux->index]) {
			path = DFS_areConnected(graph, visited, init, aux->index);
			if (path == 1) {
				return path;
			}
		}
		aux = aux->next;
	}

	return path;
}

// (pt verificare) Functie care printeaza graful sub forma de lista de adiacente
void printGraph(TGraphL *graph, FILE *file) {
    for (int i = 0; i < graph->nrNodes; i++) {
        fprintf(file, "Indexul %d: ", i);
        TNode *aux = graph->adjList[i];
        while (aux != NULL) {
            fprintf(file, "%s cu %d, ", aux->name, aux->index);
            aux = aux->next;
        }
        fprintf(file, "\n");
    }
    return;
}