#include "../lib/cppjieba/include/cppjieba/Jieba.hpp"

#include <iostream>
#include <string>
#include <vector>
using namespace std;

const char * const DICT_PATH = "../lib/cppjieba/dict/jieba.dict.utf8";//最大概率法(MPSegment: Max Probability)分词所使用的词典路径
const char * const HMM_PATH = "../lib/cppjieba/dict/hmm_model.utf8";//隐式马尔科夫模型(HMMSegment: Hidden Markov Model)分词所使用的词典路径
const char * const USER_DICT_PATH = "../lib/cppjieba/dict/user.dict.utf8";//用户自定义词典路径
const char* const IDF_PATH = "../lib/cppjieba/dict/idf.utf8";//IDF路径
const char* const STOP_WORD_PATH = "../lib/cppjieba/dict/stop_words.utf8";//停用词路径

class WordSegmentation//使用结巴分词库进行分词
{
public:
    WordSegmentation()
    :_jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH)
    {}

    vector<string> cutsmall(const string & str)
    {
        vector<string> words;
        //底层为MPsegment:基于MP切词算法
        _jieba.CutSmall(str, words,0);
        return words;
    }
    
    vector<string> cuthmm(const string & str)
    {
        vector<string> words;
        //底层为HMMsegment:基于HMM切词算法
        _jieba.CutHMM(str, words);
        return words;
    }
    
    vector<string> cut(const string & str)
    {
        vector<string> words;
        //底层为MixSegment:融合MP和HMM的切词算法,效果最好
        _jieba.Cut(str, words);
        return words;
    }
    
    vector<string> cutall(const string & str)
    {
        vector<string> words;
        //底层为FullSegment:切出所有沾边的字典里的词语
        _jieba.CutAll(str, words);
        return words;
    }
    
    vector<string> cutforSearch(const string & str)
    {
        vector<string> words;
        //底层为QuerySegment:先用Mix方法切词,对较长词再使用Full方法切
        _jieba.CutForSearch(str, words);
        return words;
    }
private:
    cppjieba::Jieba _jieba;
};
