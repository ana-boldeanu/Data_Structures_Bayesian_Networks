#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxiliare.h"

// Parcurgere DFS recursiva
int DFS_isCicle(TGraphL* graph, TNode *parentsList, int index) {
	// Retin ca nodul cu indexul respectiv a fost vizitat
	TNode *new_list = calloc(1, sizeof(TNode));
    TNode *to_free = new_list;

    new_list->index = index;
    new_list->next = parentsList;

    int isCicle = 0;

	// Apelez recursiv pentru toti vecinii
	TNode *aux = graph->adjList[index];

	while (aux != NULL && new_list != NULL) {
        TNode *tmp = new_list;  // Parcurg lista de stramosi
        while (tmp != NULL) {
            if (aux->index == tmp->index) { // A intalnit unul din stramosi
                return 1;
            }
            tmp = tmp->next;
        }

		isCicle = DFS_isCicle(graph, new_list, aux->index);
        if (isCicle == 1) {
            return 1;
        }

        new_list = new_list->next;
		aux = aux->next;
	}

    free(to_free);

	return isCicle;
}


int Cerinta2(TGraphL *graph, char **nodeNames, char *Xname, char *Yname, char *observed) {
	int nrNodes = graph->nrNodes;
	
    // Pasul 1.
	// Construiesc o lista cu indecsii nodurilor de pastrat
	// Intai adaug stramosii nodurilor X si Y
	TNode *parents_X = NULL;
	TNode *parents_Y = NULL;
	get_ancestorsList(graph, nodeNames, Xname, &parents_X);
	get_ancestorsList(graph, nodeNames, Yname, &parents_Y);

    // Lista cu indecsii nodurilor ce trebuie pastrate
	TNode *to_keep = parents_X;

    TNode *auxY = parents_Y;
	while (auxY != NULL) {
		to_keep = add_toList(to_keep, auxY->index);
		auxY = auxY->next;
	}

    killList(parents_Y);

	// Apoi adaug nodurile X si Y
	to_keep = add_toList(to_keep, get_index(nodeNames, nrNodes, Xname));
	to_keep = add_toList(to_keep, get_index(nodeNames, nrNodes, Yname));

	// Vector cu numele nodurilor de observare
    char **obsNames = calloc(nrNodes, sizeof(char *));
	int obs = 0; // Numarul nodurilor de observare

	if (observed != NULL) {
		char *token = strtok(observed, " ");
		obsNames[obs++] = token;

		while (token) {
			token = strtok(NULL, " ");
			obsNames[obs++] = token;
		}
		obs--;

		for (int j = 0; j < obs; j++) {
			char *name = obsNames[j];
			// Adaug intai nodul
			to_keep = add_toList(to_keep, get_index(nodeNames, nrNodes, name));

			// Apoi adaug stramosii
			TNode *parents_obs = NULL;
			get_ancestorsList(graph, nodeNames, name, &parents_obs);

            TNode *tmp = parents_obs;
			while (tmp != NULL) {
				to_keep = add_toList(to_keep, tmp->index);
				tmp = tmp->next;
			}

            killList(parents_obs);
		}
	}

    // Elimin nodurile care nu se afla in lista de noduri pastrate to_keep
	for (int idx = 0; idx < nrNodes; idx++) {
		if (!contains(to_keep, idx)) {
			// Iterez prin vectorul de liste de adiacente si sterg legaturile cu k
			for (int k = 0; k < nrNodes; k++) {
				if (contains(graph->adjList[k], idx)) {
					graph->adjList[k] = remove_fromList(graph->adjList[k], idx);
				}
			}
		}
	}

    // Pasul 2.
    // Verific daca exista noduri cu cel putin 2 parinti (din graful initial)
    TGraphL *inital_graph = copy_Graph(graph, nodeNames);

    TNode *aux = to_keep;
    while (aux != NULL) {
        int parents_nr;
        TNode *parentsList = get_parentsList(inital_graph, aux->index, &parents_nr);

        if (parents_nr >= 2) {
            // Daca exista, adaug cate o muchie neorientata intre ei
            for (TNode *p = parentsList; p != NULL; p = p->next) {
                for (TNode *c = p; c != NULL; c = c->next) {
                    if (p->index != c->index) {
                        add_YtoXList(graph, nodeNames[c->index], p->index, c->index);
                        add_YtoXList(graph, nodeNames[p->index], c->index, p->index);
                    }
                }
            }
        }
        killList(parentsList);

        aux = aux->next;
    }

    killGraph(inital_graph);

    // Pasul 3.
    // Elimin orientarea arcelor
    for (int k = 0; k < nrNodes; k++) {
        aux = graph->adjList[k];    // Sunt in lista de adiacenta a lui k
        while (aux != NULL) {   // Il adaug pe k in fiecare lista de adiacenta a copiilor sai
            add_YtoXList(graph, nodeNames[k], aux->index, k);
            aux = aux->next;
        }
    }

    // Pasul 4.
    // Elimin nodurile de observare si muchiile care le contin
    for (int k = 0; k < obs; k++) {
        int obs_idx = get_index(nodeNames, nrNodes, obsNames[k]);

        // Iterez prin vectorul de liste de adiacente si sterg legaturile cu obs_idx
        for (int i = 0; i < nrNodes; i++) {
            if (contains(graph->adjList[i], obs_idx)) {
                graph->adjList[i] = remove_fromList(graph->adjList[i], obs_idx);
                to_keep = remove_fromList(to_keep, obs_idx);
            }
        }

        // "Sterg" nodul din graf, golind lista lui de adiacente
        killList(graph->adjList[obs_idx]);
        graph->adjList[obs_idx] = NULL;
    }

    // Pasul 5.
    // Verific daca nodurile initiale X si Y mai sunt in graf 
    int Xidx = get_index(nodeNames, nrNodes, Xname);
    int Yidx = get_index(nodeNames, nrNodes, Yname);
    int Xexists = 0, Yexists = 0;

    aux = to_keep;
    while (aux != NULL) {
        if (aux->index == Xidx) {
            Xexists = 1;
        }
        if (aux->index == Yidx) {
            Yexists = 1;
        }
        aux = aux->next;
    }

    if (!Xexists || !Yexists) {
        return 0;   // Daca unul nu exista, sunt independente conditional
    }

    // Verific daca intre nodurile X si Y exista acum o cale
    int *visited = calloc(nrNodes, sizeof(int));
    int reach = DFS_areConnected(graph, visited, Xidx, Yidx);

    free(visited);
    free(obsNames);
    killList(to_keep);

	return reach;
}

int main() {
    FILE *in = fopen("bnet.in", "r");
    FILE *out = fopen("bnet.out", "w");

    int nrNodes, nrEdges;
    fscanf(in, "%d %d\n", &nrNodes, &nrEdges);

    // Folosesc un vector cu numele nodurilor pentru a le indexa
    char **nodeNames = calloc(nrNodes, sizeof(char *));
    for (int i = 0; i < nrNodes; i++) {
        nodeNames[i] = calloc(MAX_STRING, sizeof(char));
        fscanf(in, "%s ", nodeNames[i]);
    }

    // Creez graful
    TGraphL *graph = createGraph(nrNodes);

    // Creez listele de adiacente
    for (int i = 0; i < nrEdges; i++) {
        char *Xname = calloc(MAX_STRING, sizeof(char));
        char *Yname = calloc(MAX_STRING, sizeof(char));

        fscanf(in, "%s %s\n", Xname, Yname);

        // Determin indexul pentru nodul X
        int Xindex = get_index(nodeNames, nrNodes, Xname);
        int Yindex = get_index(nodeNames, nrNodes, Yname);

        // Adaug nodul Y in lista de adiacenta a lui X (aflata la Xindex)
        add_YtoXList(graph, Yname, Xindex, Yindex);

        free(Xname);
        free(Yname);
    }

    // Verific ce cerinta se rezolva
    int cerinta, nrInterogari;
    if (fscanf(in, "%d\n", &nrInterogari) == 1) {
        cerinta = 2;
    } else {
        cerinta = 1;
    }

    if (cerinta == 1) {
        // Cerinta 1
        // Verific daca e ciclu cu parcurgere DFS pt fiecare nod
        int cicle = 0;

        for (int i = 0; i < nrNodes; i++) {
            cicle = DFS_isCicle(graph, NULL, i);
            
            if (cicle == 1) {
                break;
            }
        }

        if (cicle == 1) {
            fprintf(out, "imposibil\n");
        } else {
            fprintf(out, "corect\n");
        }

    } else {
        // Cerinta 2
        char *Xname, *Yname, *observed;

        char fileRow[100];

        for (int i = 0; i < nrInterogari; i++) {

            fgets(fileRow, 100, in);

            Xname = strtok(fileRow, " ;|");
            Yname = strtok(NULL, " ;|");
            observed = strtok(NULL, ";|\n");

            TGraphL *graph_copy = copy_Graph(graph, nodeNames);

            int areConnected = Cerinta2(graph_copy, nodeNames, Xname, Yname, observed);

            if (areConnected == 1) {
                fprintf(out, "neindependente\n");
            } else {
                fprintf(out, "independente\n");
            }

            killGraph(graph_copy);
        }
    }

    // Dezalocare memorie
    killGraph(graph);
    fclose(in);
    fclose(out);

    for (int i = 0; i < nrNodes; i++) {
        free(nodeNames[i]);
    }
    free(nodeNames);

    return 0;
}