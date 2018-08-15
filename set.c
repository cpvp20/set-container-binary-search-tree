#include "set.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct strNode {
	Type data;
	struct strNode *left, *right, *parent;
}* Node;

struct strSet {
	struct strNode *root;	//desde la raíz llegamos a cualquier elemento :)
	int size;
	CompareFunc compareFunc;
	PrintFunc printFunc;
	DestroyFunc destroyFunc;
};

//implementacion de funciones extras no expuestas en el .h

void destroy(Node node, DestroyFunc destroyFunc){	//RECURSIVA... es llamada desde set_destroy con la raíz
	//Hace un recorrido post-orden, eliminando primero los subárboles y luego al nodo
	if(node->left != NULL) destroy(node->left, destroyFunc);	//el sub-arbol izq (recursivamente)
	if(node->right != NULL) destroy(node->right, destroyFunc);	//el sub-árbol der (recursivamente)
	if(destroyFunc != NULL) destroyFunc(node);	//liberar la memoria del dato del nodo
	free (node);	//el mero nodo
}

void print(Node node, PrintFunc printFunc){	//RECURSIVA... es llamada desde set_print con la raíz y la función printFunc del Set
	if (node != NULL)	//Si el nodo node es nulo, no se hace nada.
	{
		printf("{");	//El recorrido del árbol debe ser pre-orden.
		printFunc(node->data);	//el data de node (pasándolo a printFunc)
		print(node->left, printFunc);	//el sub-árbol izquierdo (recursivamente)
		print(node->right, printFunc);	//el sub-árbol derecho (recursivamente)
		printf("}");
	}
}

void addLink(Node parent, Node child, bool isLeftChild){
	if (isLeftChild) parent->left = child;	//A parent le asigna child como hijo izq
	else if (!isLeftChild) parent->right = child;	//A parent le asigna child der
	if (child != NULL) child->parent = parent; //se actualiza el atr padre de child con parent como nuevo valor.
}

void removeBranch(Set s, Node n, bool isLeftChild){	//se implementará todos los casos de eliminación de nodos,
	//Caso 1: n es hoja
	if (n->left == NULL && n->right == NULL)
	{
		if (n == s->root) s->root = NULL;	//la raíz de s será nulo.
		else
		{
			if (isLeftChild) addLink(n->parent, NULL, TRUE);	//el hijo izquierdo de p (el papa del nodo) será nulo
			else addLink(n->parent, NULL, FALSE);	//Si no, el hijo derecho de p será nulo
		}
		free (n);	//Eliminar de memoria a n.
	}

	//Caso 2a: n sólo tiene hijo izquierdo
	else if (n->left != NULL && n->right == NULL)
	{
		if (n == s->root) s->root = n->left;	//la raíz de s será el hijo izquierdo de n.
		else
		{
			if (isLeftChild) addLink(n->parent, n->left, TRUE);	//el hijo izq de p será el izquierdo de n
			else addLink(n->parent, n->left, FALSE);	//Si no, el derecho de p será el izquierdo de n
		}
		free (n);	//Eliminar de memoria a n.
	}
	//Caso 2b: n sólo tiene hijo derecho
	else if (n->left == NULL && n->right != NULL)
	{
		if (n == s->root) s->root = n->right;
		else
		{
			if (isLeftChild) addLink(n->parent, n->right, TRUE);	//el hijo izq de p será el der de n
			else addLink(n->parent, n->right, FALSE);	//Si no, el derecho de p será el der de n
		}
		free (n);	//Eliminar de memoria a n.
	}

	//Caso 3: n tiene 2 hijos
	else
	{
		Node m = n->left;	//Sea m el nodo con el valor más grande en el subárbol izq (puede tener hijo izquierdo o ser hoja)
		while (m->right != NULL)
			m = m->right;
		n->data = m->data;	//Escribir en n el valor de m.
		Node p = m->parent;
		//Caso 3a: el padre de m es n o la raíz de s
		if (p == n || p == s->root) addLink(p, m->left, TRUE);	//El hijo izq de p será el hijo izq de m
		//Caso 3b: el padre de m NO es n NI raíz de s
		if (p != n && p != s->root) addLink(p, m->left, FALSE);	//El hijo derecho de p será el hijo izq de m
		free (m);
	}
}

//implementacion de funciones publicas (si en el .h)

Set set_create(CompareFunc cf, PrintFunc pf, DestroyFunc df){
	Set s = (Set) malloc(sizeof(struct strSet));
	s->root = NULL;
	s->size = 0;
	s->compareFunc = cf;
	s->printFunc = pf;
	s->destroyFunc = df;
	return s;
}

int set_size(Set s){
	if (s == NULL) return -1;
	return s->size;
}

bool set_add(Set s, Type t){
	if(s == NULL) return FALSE;

	Node n = (Node) malloc(sizeof(struct strNode));	//crea nuevo nodo c el dato y asignar nulos a los hijos.
	n->data = t;
	n->left = NULL;
	n->right = NULL;

	int added = 0;

	if(s->root == NULL)	//si el arbol esta vacio
	{
		s->root = n;
		added = 1;
	}
	//else
	Node current = s->root;	//la busqueda empieza en el root
	while(!added)
	{
		int comp = s->compareFunc (t, current->data);
		if (comp == 0)// encontramos el valor a añadir.
		{
			free(n);	//Liberamos la memoria del nodo aux creado
			return FALSE;	//devolvemos falso (porque no lo pudimos añadir)
		}
		else if (comp < 0)
		{
			if (current->left == NULL)	//Si el hijo izquierdo no existe,
			{
				addLink (current, n, TRUE);	//el hijo izquierdo será el nuevo nodo [añadido]
				added = 1;
			}
			else current = current->left;	//Si no, continuamos la búsqueda por el hijo izquierdo.
		}
		else	//(devuelve positivo)
		{
			if (current->right == NULL)	//Si el hijo der no existe,
			{
				addLink(current, n, FALSE);	//el hijo der será el nuevo nodo [añadido]
				added = 1;
			}
			else current = current->right;// Si no, continuamos la búsqueda por el hijo derecho.
		}
	}	//si se sale del while, es porque lo agrego
	++(s->size);
	return TRUE;
}

bool set_contains(Set s, Type t){
	Node current = s->root;	//la busqueda empieza en el root
	while(current != NULL)	//buscar
	{
		int comp = s->compareFunc (t, current->data);
		if (comp == 0)
			return TRUE;
		else if (comp < 0)
			current = current->left;	//continuamos la búsqueda por el hijo izquierdo.
		else	//(comp > 0)
			current = current->right;	//continuamos la búsqueda por el hijo derecho.
	}	//si se sale del while, es porque lo encontro
	return FALSE;	//Si terminó el ciclo while, no encontramos el dato.
}

void set_destroy(Set s){
	destroy(s->root, s->destroyFunc);
	free (s);
}

bool set_remove(Set s, Type t){
	if(s == NULL || s->root == NULL) return FALSE;
	if (!set_contains(s,t)) return FALSE;
	Node current = s->root;	//la busqueda empieza en el root
	int found = 0, left = 1, right = 0;
	while(current != NULL && found == 0)	//buscar
	{
		int comp = s->compareFunc (t, current->data);
		if (comp == 0)
			found = 1;
		else if (comp < 0)
		{
			current = current->left;	//continuamos la búsqueda por el hijo izquierdo.
			left = 1;
			right = 0;
		}
		else	//(comp > 0)
		{
			current = current->right;	//continuamos la búsqueda por el hijo derecho.
			left = 0;
			right = 1;
		}
	}	//si se sale del while, es porque encontramos el valor a quitar y lo podemos quitar.

	if (left) removeBranch (s, current, TRUE);
	else removeBranch (s, current, FALSE);
	--(s->size);
	return TRUE;
}

void set_print(Set s){
	print(s->root, s->printFunc);
	printf("\n");
}
