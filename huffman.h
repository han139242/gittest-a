#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <QObject>
#include <QMap>
#include <QHash>
#include <QString>
#include <QVector>

/*Huffman 树节点结构*/
struct HuffmanNode {
    QChar ch;          // 叶子节点存字符，内部节点 ch = '\0'
    int freq;          // 频率
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(QChar c = QChar(), int f = 0,HuffmanNode* l = nullptr, HuffmanNode* r = nullptr)
        : ch(c), freq(f), left(l), right(r) {}
};

/* 记录 Huffman 树构建过程中的一步合并*/
struct MergeStep {
    QString leftChars;   // 左子树包含的字符集
    int leftFreq;
    QString rightChars;
    int rightFreq;
    int mergedFreq;
};

class HuffmanCodec : public QObject
{
    Q_OBJECT
public:
    explicit HuffmanCodec(QObject* parent = nullptr);
    ~HuffmanCodec();

    // 统计文本中各字符的频率
    QMap<QChar, int> buildFrequency(const QString& text);

    // 根据频率表构造 Huffman 树，并记录构造步骤
    void buildTree(const QMap<QChar, int>& freqMap);

    // 生成 Huffman 编码表
    QHash<QChar, QString> buildCodes() const;

    // 对原文编码
    QString encode(const QString& text) const;

    // 对 0/1 串译码，ok 返回是否成功
    QString decode(const QString& bits, bool* ok = nullptr) const;

    // 获取 Huffman 树根，用于可视化
    HuffmanNode* root() const { return m_root; }

    // 获取构造步骤日志
    const QVector<MergeStep>& steps() const { return m_steps; }

private:
    HuffmanNode* m_root;
    QVector<MergeStep> m_steps;

    void clearTree(HuffmanNode* node) const;

    void buildCodesRec(HuffmanNode* node,
                       const QString& prefix,
                       QHash<QChar, QString>& out) const;

    // 辅助：获取一棵子树包含的所有字符
    void collectChars(HuffmanNode* node, QString& out) const;
};

#endif // HUFFMAN_H
