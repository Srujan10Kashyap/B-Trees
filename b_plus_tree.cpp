#include "include/b_plus_tree.h"
#include <iostream>

/*
 * Helper function to decide whether current b+tree is empty
 */
bool BPlusTree::IsEmpty() const { return(size==0); }

/*
 * Helper function to check if current key being inserted already exists
 */
bool BPlusTree::checkDuplicateKey(const KeyType &key)
{
  RecordPointer dummy;
  if(GetValue(key,dummy))
  {
    std::cout<<"Key already exists"<<"\n";
    return true;
  }
  return false;
}

/*
 * Helper function to print tree size
 */
void BPlusTree::printTreeSize() const
{
  std::cout<<"Tree size: "<<size<<"\n";
}

/*
 * Helper function to print the root contents
 */
void BPlusTree::printRoot()
{
  std::cout<<"Printing Root\n";
  printNode(root);
  return;
}

/*
 * Helper function to print the node contents
 */
void BPlusTree::printNode(Node* node)
{
  if(node==nullptr or size==0)
  {
    std::cout<<"Null pointer\n";
    return;
  }
  if(node->key_num==0)
  {
    std::cout<<"Empty node\n";
    return;
  }
  std::cout<<"Printing keys: "<<"\n";
  for(int i=0;i<node->key_num;i++)
  {
    std::cout<<i+1<<" "<<node->keys[i]<<"\n";
  }
  if(!node->is_leaf)
  {
    InternalNode* nodePtr = static_cast<InternalNode*>(node);
    std::cout<<"Printing 1st key of each child: "<<"\n";
    for(int i=0;i<node->key_num+1;i++)
    {
      std::cout<<i+1<<" "<<nodePtr->children[i]->keys[0]<<"\n";
    }
  }
  return;
}

/*
 * Helper function to insert the key value pair in the leaf node
 */
Node* BPlusTree::insertIntoLeaf(Node* c,const KeyType &key, const RecordPointer &value)
 {
   int insertIndex = 0;
   for(int i=0;i<c->key_num;i++)
   {
     if(key<c->keys[i])
     {
       insertIndex=i;
       break;
     }
     if(i==c->key_num-1)
     {
       insertIndex = c->key_num;
       break;
     }
   }

   //Shift nodes
   LeafNode* leaf = static_cast<LeafNode*>(c);
   for(int i = c->key_num;i>insertIndex;i--)
   {
     c->keys[i] = c->keys[i-1];
     leaf->pointers[i] = leaf->pointers[i-1];
   }
   c->keys[insertIndex] = key;
   leaf->pointers[insertIndex] = value;
   c->key_num+=1;
   return c;
 }

 /*
  * Helper function to traverse the BPlusTree and find the node corresponding to key
  */
Node* BPlusTree::findNode(Node* startNode, const KeyType &key)
{
  if(startNode==nullptr or size==0)
  {
    return nullptr;
  }
  Node*c = startNode;
  while(!c->is_leaf)
  {
    InternalNode* nodePtr = static_cast<InternalNode*>(c);
    for(int i=0;i<c->key_num;i++)
    {
      if(key<c->keys[i])
      {
        c = nodePtr->children[i];
        break;
      }
      if(i==(c->key_num)-1)
      {
        c = nodePtr->children[i+1];
        break;
      }
    }
  }

  return c;
}
/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
bool BPlusTree::GetValue(const KeyType &key, RecordPointer &result)
{ if(IsEmpty())
  {
    std::cout<<"Tree is empty"<<"\n";
    return false;
  }
  Node* c = root;
  while(!c->is_leaf)
  {

    InternalNode* nodePtr = static_cast<InternalNode*>(c);
    for(int i=0;i<c->key_num;i++)
    {
      if(key<c->keys[i])
      {
        c = nodePtr->children[i];
        break;
      }
      if(i==(c->key_num)-1)
      {
        c = nodePtr->children[i+1];
        break;
      }
    }
  }
  for(int i=0;i<c->key_num;i++)
  {
    if(c->keys[i]==key)
    {
      LeafNode* leaf = static_cast<LeafNode*>(c);
      result = leaf->pointers[i];
      //std::cout<<"Key found: "<<key<<"\n";
      return true;
    }
  }
  //std::cout<<"Key not found: "<<key<<"\n";
  return false;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * If current tree is empty, start new tree, otherwise insert into leaf Node.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
bool BPlusTree::Insert(const KeyType &key, const RecordPointer &value)
{
  if(checkDuplicateKey(key)) return false;

  if(IsEmpty())
  {
      LeafNode* L = new LeafNode();
      L->keys[L->key_num] = key;
      L->pointers[L->key_num] = value;
      L->key_num+=1;
      size+=1;
      root = L;
      return true;
  }
  else
  {
    Node* c;
    c = findNode(root,key);
    if(c->key_num<MAX_FANOUT-1)
    {
      c= insertIntoLeaf(c,key,value);
      size+=1;
      return true;

    }
    else
    {
      Node* newNode = new LeafNode();
      newNode->parent = c->parent;
      KeyType key_copy[MAX_FANOUT];
      for(int i=0;i<c->key_num;i++)
      {
        key_copy[i] = c->keys[i];
        c->keys[i] = 0;
      }
      RecordPointer pointer_copy[MAX_FANOUT];
      LeafNode* nodePtr = static_cast<LeafNode*>(c);
      for(int i=0;i<c->key_num;i++)
      {
          pointer_copy[i] = nodePtr->pointers[i];
      }

      int len = c->key_num;
      int insertIndex = 0;
      for(int i=0;i<len;i++)
      {
        if(key<key_copy[i])
        {
          insertIndex = i;
          break;
        }
        if(i==len-1)
        {
          insertIndex = len;
          break;
        }
      }
      for(int i=len;i>insertIndex;i--)
      {
        key_copy[i] = key_copy[i-1];
        pointer_copy[i] = pointer_copy[i-1];
      }
      key_copy[insertIndex] = key;
      pointer_copy[insertIndex] = value;
      c->key_num = MAX_FANOUT/2;
      if(MAX_FANOUT%2==0)
      {
        newNode->key_num = MAX_FANOUT/2;
      }
      else{
        newNode->key_num = (MAX_FANOUT/2)+1;
      }
      for(int i=0;i<c->key_num;i++)
      {

        c->keys[i] = key_copy[i];
        nodePtr->pointers[i] = pointer_copy[i];
      }
      LeafNode* newNodePtr = static_cast<LeafNode*>(newNode);
      for(int j=0;j<newNode->key_num;j++)
      {
        newNode->keys[j] = key_copy[j+c->key_num];
        newNodePtr->pointers[j] = pointer_copy[j+c->key_num];
      }

      newNodePtr->next_leaf = nodePtr->next_leaf;
      newNodePtr->prev_leaf = nodePtr;
      nodePtr->next_leaf = newNodePtr;
      nodePtr->next_leaf->prev_leaf = nodePtr;
      KeyType kPrime = key_copy[MAX_FANOUT/2];
      if(InsertIntoParent(c,newNode,kPrime))
      {
        size+=1;
        return true;
      }
      else{
        std::cout<<"Failed";
        return false;
      }


    }

  }
  return false;
}

/*
 * Helper function to link new child to parent
 */
bool BPlusTree::InsertIntoParent(Node* parent,Node* child,const KeyType &kPrime)
{
  if(parent==root)
  {

    Node* newRoot = new InternalNode();
    parent->parent = newRoot;
    child->parent = newRoot;
    InternalNode* newRootPtr = static_cast<InternalNode*>(newRoot);
    newRootPtr->children[0] = parent;
    newRootPtr->children[1] = child;
    newRoot->keys[0] = kPrime;
    newRoot->key_num+=1;
    root = newRoot;
    return true;
  }
  else{
    parent = parent->parent;
    if(parent->key_num<MAX_FANOUT-1)
    {
          int insertIndex = 0;
          int len = parent->key_num;
          for(int i=0;i<parent->key_num;i++)
          {
            if(kPrime<parent->keys[i])
            {
              insertIndex = i;
              break;
            }
            if(i==parent->key_num-1)
            {
              insertIndex = parent->key_num;
              break;
            }
          }
          InternalNode* parentPtr = static_cast<InternalNode*>(parent);
          for(int j=parent->key_num;j>insertIndex;j--)
          {
            parent->keys[j] = parent->keys[j-1];
          }
          for(int j=len+1;j>insertIndex+1;j--)
          {
            parentPtr->children[j] = parentPtr->children[j-1];
          }
          parent->keys[insertIndex] = kPrime;
          parentPtr->children[insertIndex+1] = child;
          parent->key_num+=1;
          return true;
    }
    else
    {
          Node* newNode = new InternalNode();
          if(parent->parent!=nullptr)
          {
            newNode->parent = parent->parent;
          }
          KeyType key_copy[MAX_FANOUT];
          for(int i=0;i<parent->key_num;i++)
          {
            key_copy[i] = parent->keys[i];
          }
          int len = parent->key_num;
          int insertIndex = 0;
          for(int i=0;i<parent->key_num;i++)
          {
            if(kPrime < parent->keys[i])
            {

              insertIndex = i;
              break;
            }
            if(i==parent->key_num-1)
            {
              insertIndex = parent->key_num;
              break;
            }
          }

          for(int j=len;j>insertIndex;j--)
          {
            key_copy[j] = key_copy[j-1];
          }

          key_copy[insertIndex] = kPrime;
          Node* child_copy[MAX_FANOUT+1];
          InternalNode* parentPtr = static_cast<InternalNode*>(parent);
          for(int i=0;i<parent->key_num+1;i++)
          {
            child_copy[i] = parentPtr->children[i];
          }
          for(int j=len+1;j>insertIndex+1;j--)
          {
            child_copy[j] = child_copy[j-1];
          }
          child_copy[insertIndex+1] = child;


          parent->key_num = MAX_FANOUT/2;
          if(MAX_FANOUT%2==0)
          {
            newNode->key_num = (MAX_FANOUT/2)-1;
          }
          else
          {
            newNode->key_num = MAX_FANOUT/2;
          }

          for(int i=0;i<parent->key_num;i++)
          {
            parent->keys[i] = key_copy[i];
            parentPtr->children[i] = child_copy[i];
          }
          parentPtr->children[parent->key_num] = child_copy[parent->key_num];
          KeyType kDoublePrime = key_copy[MAX_FANOUT/2];

          InternalNode* newNodePtr = static_cast<InternalNode*>(newNode);
          for(int j=0;j<newNode->key_num;j++)
          {
            newNode->keys[j] = key_copy[parent->key_num+j+1];
            newNodePtr->children[j] = child_copy[parent->key_num+j+1];
            newNodePtr->children[j]->parent = newNode;
          }
          newNodePtr->children[newNode->key_num] = child_copy[newNode->key_num+parent->key_num+1];
          newNodePtr->children[newNode->key_num]->parent = newNode;
          if(InsertIntoParent(parent,newNode,kDoublePrime))
          {
            return true;
          }

    }
  }
  return false;
}
/*
 * Helper function to remove key from Internal Nodes after deleted from leaf node
 */
void BPlusTree::removeKeyFromParent(Node* curr,KeyType const &key,KeyType const &newKey)
{
  if(curr!=nullptr)
  {
    for(int i=0;i<curr->key_num;i++)
    {
      if(curr->keys[i]==key)
      {
        curr->keys[i]=newKey;
        return;
      }
      removeKeyFromParent(curr->parent,key,newKey);
    }
  }

  return;
}
/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf node as deletion target, then
 * delete entry from leaf node. Remember to deal with redistribute or merge if
 * necessary.
 */
void BPlusTree::Remove(const KeyType &key)
{
  Node* curr;
  curr = findNode(root,key);

  if(curr==nullptr)
  {
    std::cout<<"Nullptr, Key not found for key "<<key<<"\n";
    return;
  }
  KeyType deleteIndex = -1;
  for(int i=0;i<curr->key_num;i++)
  {
    if(curr->keys[i]==key)
    {
      deleteIndex = i;
      break;
    }
  }
  if(deleteIndex==-1)
  {
    std::cout<<curr->key_num<<"\n";
    printNode(curr);
    std::cout<<"Key not found for key "<<key<<"\n";
    return;
  }
  LeafNode* currLeafPtr = static_cast<LeafNode*>(curr);
  for(int i=deleteIndex;i<curr->key_num-1;i++)
  {
    curr->keys[i] = curr->keys[i+1];
    currLeafPtr->pointers[i] = currLeafPtr->pointers[i+1];
  }
  size-=1;
  curr->keys[curr->key_num-1]=0;
  currLeafPtr->pointers[curr->key_num-1] = RecordPointer(0,0);
  curr->key_num-=1;

  if(curr==root)
  {
    if(curr->key_num==0){
      root = nullptr;
    }
    return;
  }
  //Case when the key of a leaf node is deleted but key exists in the  parent above
  removeKeyFromParent(curr->parent,key,curr->keys[0]);

  InternalNode* parentPtr = static_cast<InternalNode*>(curr->parent);
  KeyType nodeIndex = -1;
  for(int i=0;i<curr->parent->key_num+1;i++)
  {
    if(curr == parentPtr->children[i])
    {
      nodeIndex = i;
      break;
    }
  }
  KeyType left = nodeIndex-1;
  KeyType right = nodeIndex+1;
  if(curr->key_num < MAX_FANOUT/2)
  {

        if(left>=0)
        {

            Node* leftSib = parentPtr->children[left];
            LeafNode* leftSibPtr = static_cast<LeafNode*>(leftSib);
            if(leftSib->key_num > MAX_FANOUT/2)
            {

                for(int i=curr->key_num;i>0;i--)
                {
                  curr->keys[i] = curr->keys[i-1];
                  currLeafPtr->pointers[i] = currLeafPtr->pointers[i-1];
                }
                curr->keys[0] = leftSib->keys[leftSib->key_num-1];
                currLeafPtr->pointers[0] = leftSibPtr->pointers[leftSib->key_num-1];
                curr->key_num+=1;
                leftSib->keys[leftSib->key_num-1] = 0;
                leftSibPtr->pointers[leftSib->key_num-1].page_id = 0;
                leftSibPtr->pointers[leftSib->key_num-1].record_id = 0;
                leftSib->key_num-=1;
                curr->parent->keys[left] = curr->keys[0];

                return;
            }
        }
        if(right<=curr->parent->key_num)
        {
          Node* rightSib = parentPtr->children[right];
          LeafNode* rightSibPtr = static_cast<LeafNode*>(rightSib);
          if(rightSib->key_num>MAX_FANOUT/2)
          {

            curr->keys[curr->key_num] = rightSib->keys[0];
            currLeafPtr->pointers[curr->key_num] = rightSibPtr->pointers[0];
            curr->key_num+=1;
            for(int i=1;i<rightSib->key_num+1;i++)
            {
              rightSib->keys[i-1] = rightSib->keys[i];
              rightSibPtr->pointers[i-1] = rightSibPtr->pointers[i];
            }

            rightSib->keys[rightSib->key_num-1] = 0;
            rightSibPtr->pointers[rightSib->key_num-1] = RecordPointer(0,0);

            rightSib->key_num-=1;
            curr->parent->keys[right-1] = rightSib->keys[0];
            return;

          }
        }

        if(left>=0)
        {

          Node* leftSib = parentPtr->children[left];
          LeafNode* leftSibPtr = static_cast<LeafNode*>(leftSib);
          //Copy curr node to left node.
          for(int i=0;i<curr->key_num;i++)
          {
            leftSib->keys[leftSib->key_num+i] = curr->keys[i];
            leftSibPtr->pointers[leftSib->key_num+i] = currLeafPtr->pointers[i];
          }

          leftSib->key_num = leftSib->key_num + curr->key_num;
          leftSibPtr->next_leaf = currLeafPtr->next_leaf;
          if(currLeafPtr->next_leaf!=nullptr)
          {
            currLeafPtr->next_leaf->prev_leaf = leftSibPtr;
          }
          RemoveFromParent(curr,left,curr->parent);
          return;

        }
        if(right<=curr->parent->key_num)
        {
          Node* rightSib = parentPtr->children[right];
          LeafNode* rightSibPtr = static_cast<LeafNode*>(rightSib);
          //Copy right node to curr
          for(int i=0;i<rightSib->key_num;i++)
          {
            curr->keys[curr->key_num + i] = rightSib->keys[i];
            currLeafPtr->pointers[curr->key_num + i] = rightSibPtr->pointers[i];
          }
          curr->key_num = curr->key_num + rightSib->key_num;
          currLeafPtr->next_leaf = rightSibPtr->next_leaf;
          if(rightSibPtr->next_leaf!=nullptr)
          {
            rightSibPtr->next_leaf->prev_leaf = currLeafPtr;
          }
          RemoveFromParent(rightSib,right-1,curr->parent);
          return;
        }
  }
//end of REMOVE
}

/*
 * Helper function to remove child from parent
 */
void BPlusTree::RemoveFromParent(Node* remNode, KeyType index, Node* curr)
{

  InternalNode* currInternalPtr = static_cast<InternalNode*>(curr);
  if(curr==root && curr->key_num==1)
  {
        if(remNode==currInternalPtr->children[0])
        {
          root = currInternalPtr->children[1];
          return;
        }
        else if(remNode==currInternalPtr->children[1])
        {
          root = currInternalPtr->children[0];
          return;
        }
  }
  
  for(int i=index;i<curr->key_num-1;i++)
  {
    curr->keys[i] = curr->keys[i+1];
  }
  curr->keys[curr->key_num-1] = 0;
  KeyType remIndex = -1;

  for(int i=0;i<curr->key_num+1;i++)
  {
    if(currInternalPtr->children[i]==remNode)
    {
      remIndex = i;
      break;
    }
  }
  if(remIndex==-1)
  {
    std::cout<<"Node to be removed not found among children\n";
    return;
  }
  for(int i=remIndex;i<curr->key_num;i++)
  {
    currInternalPtr->children[i] = currInternalPtr->children[i+1];
  }
  currInternalPtr->children[curr->key_num] = nullptr;
  curr->key_num-=1;
  if(curr->key_num+1< MAX_FANOUT/2)
  {
    KeyType nodeIndex = -1;
    InternalNode* parentPtr = static_cast<InternalNode*>(curr->parent);

    for(int i=0;i<curr->parent->key_num+1;i++)
    {
      if(curr == parentPtr->children[i])
      {
        nodeIndex = i;
        break;
      }
    }
    KeyType left = nodeIndex-1;
    KeyType right = nodeIndex+1;

            if(left>=0)
            {
              Node* leftSib = parentPtr->children[left];
              InternalNode* leftSibPtr = static_cast<InternalNode*>(leftSib);
              if(leftSib->key_num>MAX_FANOUT/2)
              {

                for(int i=curr->key_num;i>0;i--)
                {
                  curr->keys[i] = curr->keys[i-1];
                }
                curr->keys[0] = curr->parent->keys[left];
                curr->parent->keys[left] = leftSib->keys[leftSib->key_num-1];
                leftSib->keys[leftSib->key_num-1] = 0;
                for(int i=curr->key_num+1;i>0;i--)
                {
                  currInternalPtr->children[i] = currInternalPtr->children[i-1];
                }
                currInternalPtr->children[0] = leftSibPtr->children[leftSib->key_num];
                leftSibPtr->children[leftSib->key_num] = NULL;
                curr->key_num++;
                leftSib->key_num--;

                return;
              }

            }

            if(right<=curr->parent->key_num)
            {
              Node* rightSib = parentPtr->children[right];
              InternalNode* rightSibPtr = static_cast<InternalNode*>(rightSib);
              if(rightSib->key_num > MAX_FANOUT/2)
              {
                currInternalPtr->children[curr->key_num+1] = rightSibPtr->children[0];
                curr->keys[curr->key_num] = curr->parent->keys[right-1];
                curr->parent->keys[right-1] = rightSib->keys[0];
                for(int i=0;i<rightSib->key_num+1;i++)
                {
                  rightSib->keys[i] = rightSib->keys[i+1];
                }
                for(int i=0;i<rightSib->key_num+2;i++)
                {
                  rightSibPtr->children[i] = rightSibPtr->children[i+1];
                }
                rightSib->key_num--;
                curr->key_num++;

                return;
              }
            }

            if(left>=0)
            {

              Node* leftSib = parentPtr->children[left];
              InternalNode* leftSibPtr = static_cast<InternalNode*>(leftSib);
              leftSib->keys[leftSib->key_num] = curr->parent->keys[left];
              for(int i=0;i<curr->key_num;i++)
              {
                leftSib->keys[leftSib->key_num+i+1] = curr->keys[i];
              }
              for(int i=0;i<curr->key_num+1;i++)
              {
                leftSibPtr->children[leftSib->key_num+1+i] = currInternalPtr->children[i];
                currInternalPtr->children[i]->parent = leftSib;
                currInternalPtr->children[i] = nullptr;
              }
              leftSib->key_num = leftSib->key_num + curr->key_num+1;
              RemoveFromParent(curr,left,curr->parent);
              return;
            }

            if(right<=curr->parent->key_num)
            {
              Node* rightSib = parentPtr->children[right];
              InternalNode* rightSibPtr = static_cast<InternalNode*>(rightSib);
              curr->keys[curr->key_num] = curr->parent->keys[right-1];
              for(int i=0;i<rightSib->key_num;i++)
              {
                curr->keys[curr->key_num + i+1] = rightSib->keys[i];
              }
              for(int i=0;i<rightSib->key_num+1;i++)
              {
                currInternalPtr->children[curr->key_num + 1 + i] = rightSibPtr->children[i];
                rightSibPtr->children[i]->parent = curr;
                rightSibPtr->children[i] = nullptr;
              }
              curr->key_num = curr->key_num + rightSib->key_num+1;

              RemoveFromParent(rightSib,right-1,curr->parent);
              return;

            }
  }

}
/*****************************************************************************
 * RANGE_SCAN
 *****************************************************************************/
/*
 * Return the values that within the given key range
 * First find the node large or equal to the key_start, then traverse the leaf
 * nodes until meet the key_end position, fetch all the records.
 */
void BPlusTree::RangeScan(const KeyType &key_start, const KeyType &key_end,std::vector<RecordPointer> &result)
{

  Node* startNode = findNode(root,key_start);
  if(startNode==nullptr) return;

  KeyType currKey = startNode->keys[0];
  LeafNode* cursor_Ptr = static_cast<LeafNode*>(startNode);
  while(currKey<key_end and cursor_Ptr!=nullptr)
  {
    if(cursor_Ptr==nullptr) break;

    for(int i=0;i<cursor_Ptr->key_num;i++)
    {
      currKey = cursor_Ptr->keys[i];
      if((currKey>=key_start)&&(currKey<key_end))
      {
        result.push_back(cursor_Ptr->pointers[i]);
      }

    }
    cursor_Ptr = cursor_Ptr->next_leaf;
  }

  return;
}
