# understanding underlying algorithem: https://towardsdatascience.com/tf-idf-for-document-ranking-from-scratch-in-python-on-real-world-dataset-796d339a4089
# https://programminghistorian.org/en/lessons/analyzing-documents-with-tfidf#2-min_df-max_df 
import time
start = time.time()
import numpy as np 
import os
from collections import defaultdict
import re
import array
from threading import Thread


# TODO: Give proper weight to the title + author (which I currently ignore)



# here you specify which folders do you want to index and filters (to filter important files to index with unimportant ones)
roots_and_filters = [
  ['C:\\Users\\bkand\\Downloads\\', lambda x: x.endswith('.pdf')],
  ['E:\\imp\\', lambda x: True],
  ['E:\\archivebox\\archive\\', lambda x: x.endswith('.html') and x.find('.orig')==-1 and x.find('woff2')==-1]
]

CACHE_FILENAME = 'index_cache.npz'






class LoadFileNamesThread(Thread):
  def __init__(self, roots_and_filters, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.roots_and_filters = roots_and_filters
  def run(self):
    self.all_files = []
    for name, filter in self.roots_and_filters:
      for p, d, f in os.walk(name):
        for file in f:
          if filter(file): 
            self.all_files.append(os.path.join(p , file))

class LoadIndexThread(Thread):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
  def run(self):
    cache = np.load(CACHE_FILENAME)
    cache.allow_pickle=True

    self.m = cache['matrix']
    self.fts = dict(zip(cache['files'], cache['timestamps']))
    self.v = {}
    for i, word in enumerate(cache['vocab']): 
      self.v[word] = i

def save_index(files, timestamps, vocabulary, matrix):
  assert len(files) == len(timestamps)
  # print('not saving')
  np.savez_compressed(CACHE_FILENAME, 
    files=np.asarray(files), timestamps=np.asarray(timestamps),
    vocab=np.asarray(list(vocabulary.keys())), matrix=matrix
  )
  # thread = Thread(
  #   target=np.savez_compressed, 
  #   args=(CACHE_FILENAME),
  #   kwargs={
  #     'files' : np.asarray(files), 
  #     'timestamps' : np.array(timestamps, dtype=np.float32), 
  #     'vocab' : np.asarray(list(vocabulary.keys())), 
  #     'matrix' : matrix, 
  #   },
  #   daemon=True
  # )
  # thread.start()

if not os.path.exists(CACHE_FILENAME):
  save_index([], [], {}, np.zeros((0,0)))

# Loading cached index and getting all the file names on the system
t1 = LoadFileNamesThread(roots_and_filters)
t2 = LoadIndexThread()
t1.start(); t2.start()
t2.join(); t1.join()
all_files = t1.all_files
fts, v, m = t2.fts, t2.v, t2.m

# checking whether we need to change index
new_files = []
modified_files = []
for f in all_files: 
  timestamp = fts.get(f)
  if timestamp: 
    if timestamp != os.path.getmtime(f):
      modified_files.append(f)
      print('mod', f)
  else: 
    if not f in fts.keys():
      print('new', f)
      new_files.append(f)

deleted_files = []
for f in fts.keys():
  if not f in all_files: 
    print('del', f)
    deleted_files.append(f)

files = list(fts.keys())

def extract_text_from_files(filenames):
  import fitz
  from bs4 import BeautifulSoup

  documents = []
  for f in filenames: 
    if f.endswith('.html'): 
      try: 
        with open(f, 'r', encoding='utf8') as html: 
            content = html.read()
            soup = BeautifulSoup(content, 'html.parser')
            paragraphs = soup.find_all('p')
            if len(paragraphs) > 0:
              documents.append(' '.join([p.text for p in soup.find_all('p')]) + 
                              ' '.join([p.text for p in soup.find_all('div')]) + 
                              ' '.join([p.text for p in soup.find_all('h1')]) + 
                              ' '.join([p.text for p in soup.find_all('h2')]))

      except: 
        continue
    elif f.endswith('.pdf'): 
        doc = fitz.open(f)
        documents.append(' '.join([p.get_text() for p in doc]))
    else: 
      with open(f, 'r', encoding='utf8') as file: 
        documents.append(file.read())
    # print(f)
  return documents

# source: "from nltk.corpus import stopwords; stopwords.words('english')"
stop_words = [ 
  'i', 'me', 'my', 'myself', 'we', 'our', 'ours', 'ourselves', 'you',
 "you're", "you've", "you'll", "you'd", 'your', 'yours', 'yourself', 'yourselves',
 'he', 'him', 'his', 'himself', 'she', "she's", 'her', 'hers', 'herself', 'it',
 "it's", 'its', 'itself', 'they', 'them', 'their', 'theirs', 'themselves', 'what',
 'which', 'who', 'whom', 'this', 'that', "that'll", 'these', 'those', 'am', 'is',
 'are', 'was', 'were', 'be', 'been', 'being', 'have', 'has', 'had', 'having', 'do',
 'does', 'did', 'doing', 'a', 'an', 'the', 'and', 'but', 'if', 'or', 'because', 'as',
 'until', 'while', 'of', 'at', 'by', 'for', 'with', 'about', 'against', 'between',
 'into', 'through', 'during', 'before', 'after', 'above', 'below', 'to', 'from',
 'up', 'down', 'in', 'out', 'on', 'off', 'over', 'under', 'again', 'further', 'then',
 'once', 'here', 'there', 'when', 'where', 'why', 'how', 'all', 'any', 'both', 'each',
 'few', 'more', 'most', 'other', 'some', 'such', 'no', 'nor', 'not', 'only', 'own',
 'same', 'so', 'than', 'too', 'very', 's', 't', 'can', 'will', 'just', 'don', "don't",
 'should', "should've", 'now', 'd', 'll', 'm', 'o', 're', 've', 'y', 'ain', 'aren',
 "aren't", 'couldn', "couldn't", 'didn', "didn't", 'doesn', "doesn't", 'hadn',
 "hadn't", 'hasn', "hasn't", 'haven', "haven't", 'isn', "isn't", 'ma', 'mightn',
 "mightn't", 'mustn', "mustn't", 'needn', "needn't", 'shan', "shan't", 'shouldn',
 "shouldn't", 'wasn', "wasn't", 'weren', "weren't", 'won', "won't", 'wouldn', "wouldn't"
]

def tokenize(text):
  return re.findall(r"(?u)\b[a-zA-Z_][a-zA-Z_]+\b|\d+", text.lower())

def remove_stop_words(tokens, stop_wrods):
  inconsistent = []
  for token in tokens:
      if token not in stop_wrods:
        inconsistent.append(token)
  return inconsistent

# _stopwords = list(np.unique(tokenize(' '.join(stop_words))))

def vectorize(documents, vocabulary={}, grow_vocab=False):
  
  if len(vocabulary) == 0 or grow_vocab: 
    # Add a new value when a new vocabulary item is seen
    vocabulary = defaultdict(None, vocabulary)
    vocabulary.default_factory = vocabulary.__len__

  j_indices = []
  indptr = []
  values = array.array(str("i"))

  indptr.append(0)
  for i, doc in enumerate(documents): 
    
      tokens = tokenize(doc) # turn into tokens
      tokens = remove_stop_words(tokens, stop_words)

      feature_counter = defaultdict(lambda:0)
      for token in tokens:
        try: 
          token_idx = vocabulary[token]
          if token_idx:
            feature_counter[token_idx] += 1
        except: 
          continue
      j_indices.extend(feature_counter.keys())
      values.extend(feature_counter.values())
      indptr.append(len(j_indices))

  vocabulary = dict(vocabulary) # disable default dict behaviour

  indices_dtype = np.int32
  j_indices = np.asarray(j_indices, dtype=indices_dtype)
  indptr = np.asarray(indptr, dtype=indices_dtype)
  values = np.frombuffer(values, dtype=np.intc)
  
  # NOTE: not needed anymore? 
  # X = sp.csr_matrix(
  #     (values, j_indices, indptr),
  #     shape=(len(indptr) - 1, len(vocabulary)),
  #     dtype=np.float32
  # )
  # X.sort_indices()
  # C = X.toarray()

  for i, j0, j1 in zip(range(len(indptr)), indptr, indptr[1:]): 
    j_indices[j0:j1] += i*len(vocabulary)
  shape = (len(indptr) - 1, len(vocabulary))
  X = np.zeros(shape, dtype=np.float32)
  np.put(X, j_indices, values)

  return X, vocabulary

def calc_tf_idf(X):
  N = X.shape[0]
  tf  = X / (X.sum(axis=1, keepdims=True)+0.1)
  df  = (X > 0).sum(axis=0, keepdims=True)
  idf = np.log10(N / (1+df))+1
  return tf * idf



# Change index if needed

old_mat = m
if len(new_files) > 0:
  # create count matrix for new files 
  new_documents = extract_text_from_files(new_files)
  new_mat, new_vocabulary = vectorize(new_documents, vocabulary=v, grow_vocab=True)

  # update tf-idf matrix
  c = np.c_[old_mat, np.zeros((old_mat.shape[0], new_mat.shape[1]-old_mat.shape[1]))]
  count_matrix = np.r_[c, new_mat]
  old_mat = count_matrix

  # update vocabulary 
  new_idx = len(v.keys())
  for word in new_vocabulary.keys(): 
    idx = v.get(word)
    if idx is None:
      v[word] = new_idx
      new_idx += 1

  # update file names
  files += new_files

if len(deleted_files) > 0: 
  # remove unwanted columns from old tf-idf matrix 
  idxs = [files.index(df) for df in deleted_files]
  count_matrix = np.delete(old_mat, idxs, axis=0)   
  old_mat = count_matrix

  # update file names 
  for idx in idxs:
    files.remove(files[idx])

if len(modified_files) > 0:
  # create count matrix for new files 
  mod_documents = extract_text_from_files(modified_files)
  mod_mat, mod_vocabulary = vectorize(mod_documents, vocabulary=v)

  # update tf-idf matrix
  count_matrix = old_mat 
  # mod_mat = mod_mat.toarray()
  idxs = [files.index(mf) for mf in modified_files]
  for i, idx in enumerate(idxs):
    count_matrix[idx] = mod_mat[i]

  # update vocabulary 
  mod_idx = len(v.keys())
  for word in mod_vocabulary.keys(): 
    idx = v.get(word)
    if idx is None:
      v[word] = mod_idx
      mod_idx += 1

# Save changes
if len(new_files + deleted_files + modified_files) > 0: 
  tf_idf = calc_tf_idf(count_matrix)
  print('saving changes')
  timestamps = [os.path.getmtime(f) for f in files]
  save_index(files, timestamps, v, count_matrix)
else: 
  tf_idf = calc_tf_idf(m)





# jaro-winkler distance for fuzzy matching
# Copied from: https://www.geeksforgeeks.org/jaro-and-jaro-winkler-similarity/
# contributed by AnkitRai010
def jaro_distance(s1, s2):
 
    if (s1 == s2):
      return 1.0
 
    len1, len2 = len(s1), len(s2)
    if (len1 == 0 or len2 == 0):
      return 0.0
 
    max_dist = (max(len(s1), len(s2)) // 2 ) - 1
    match_count = 0
 
    # Hash for matches
    hash_s1 = [0] * len(s1)
    hash_s2 = [0] * len(s2)
 
    # Traverse through the first string
    for i in range(len1) :
      # Check if there is any matches
      for j in range( max(0, i - max_dist), min(len2, i + max_dist + 1)) :
        if (s1[i] == s2[j] and hash_s2[j] == 0) :
          hash_s1[i] = 1
          hash_s2[j] = 1
          match_count += 1
          break
      
    if (match_count == 0):
      return 0.0
    
    t = 0 # Number of transpositions
    point = 0
 
    # Count number of occurrences
    # where two characters match but
    # there is a third matched character
    # in between the indices
    for i in range(len1) :
      if (hash_s1[i]) :
        # Find the next matched character
        # in second string
        while (hash_s2[point] == 0):
          point += 1

        if (s1[i] != s2[point]):
          point += 1
          t += 1
        else :
          point += 1
              
      t /= 2
 
    # Return the Jaro Similarity
    return ((match_count / len1 + match_count / len2 +
            (match_count - t) / match_count) / 3.0)
 
def jaro_Winkler(s1, s2) :
  jaro_dist = jaro_distance(s1, s2)

  if (jaro_dist > 0.7) :

    # Find the length of common prefix
    prefix = 0

    for i in range(min(len(s1), len(s2))) :
      
      if (s1[i] == s2[i]) :
        prefix += 1
      else :
        break

    prefix = min(4, prefix)

    # Calculate jaro winkler Similarity
    jaro_dist += 0.1 * prefix * (1 - jaro_dist)

  return jaro_dist

# Use resulting tf-idf matrix to compare query to other files 
def print_files_from_query(X, query, vocabulary={}):

  Q, _ = vectorize([query], vocabulary=vocabulary)
  Q = calc_tf_idf(Q)

  def cosine_sim(a, b):
    d = np.linalg.norm(a)*np.linalg.norm(b)
    cos_sim = np.dot(a, b)/d if d > 0 else 0
    return cos_sim

  sim = {}
  # Calculate the similarity
  for i in range(X.shape[0]):
    sim[i] = cosine_sim(Q, X[i])

  # Sort the values 
  sim_sorted = sorted(sim.items(), key=lambda x: x[1], reverse=True)

  # Print the articles and their similarity values
  for k, v in sim_sorted:
    if v != 0.0 and not np.isnan(v):
      print(f"Nilai Similaritas: {v.item():.4f} - {files[k].split('/')[-1]}")



import sys
if len(sys.argv)>1: 
  query = sys.argv[1]
  result = query

  tokens = remove_stop_words(query.split(), stop_words)
  scores = []
  for t in tokens: 
    for word in list(v.keys()):
      if word.find(t) == -1: continue
      score = jaro_Winkler(t, word) 
      if score > 0.88: scores.append((word, score))
  result = sorted(scores, key=lambda x: x[1], reverse=True)
  query = ' '.join(map(lambda x: x[0], result))
  # print(query)
  print_files_from_query(tf_idf, query, vocabulary=v)

print(f'total time spent: {time.time()-start:.4f}')

