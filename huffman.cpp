#include "huffman.h"
#include <queue>

struct NodeCmp {
    bool operator()(HuffmanNode* a, HuffmanNode* b) const {
        return a->freq > b->freq; // 小顶堆
    }
};

HuffmanCodec::HuffmanCodec(QObject* parent)
    : QObject(parent),
    m_root(nullptr)
{
}

HuffmanCodec::~HuffmanCodec()
{
    clearTree(m_root);
    m_root = nullptr;
}

void HuffmanCodec::clearTree(HuffmanNode* node) const
{
    if (!node) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

QMap<QChar, int> HuffmanCodec::buildFrequency(const QString& text)
{
    QMap<QChar, int> freq;
    for (QChar c : text) {
        freq[c] += 1;
    }
    return freq;
}

void HuffmanCodec::collectChars(HuffmanNode* node, QString& out) const
{
    if (!node) return;
    if (!node->left && !node->right) {
        out.append(node->ch);
        return;
    }
    collectChars(node->left, out);
    collectChars(node->right, out);
}

void HuffmanCodec::buildTree(const QMap<QChar, int>& freqMap)
{
    // 清理旧树和步骤
    clearTree(m_root);
    m_root = nullptr;
    m_steps.clear();

    if (freqMap.isEmpty())
        return;

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeCmp> heap;
    for (auto it = freqMap.constBegin(); it != freqMap.constEnd(); ++it) {
        HuffmanNode* node = new HuffmanNode(it.key(), it.value());
        heap.push(node);
    }

    if (heap.size() == 1) {
        HuffmanNode* only = heap.top();
        heap.pop();
        m_root = new HuffmanNode(QChar(), only->freq, only, nullptr);

        // 记录一步（只有一个字符的情况，可以当作合并自己）
        MergeStep step;
        QString chs;
        chs.append(only->ch);
        step.leftChars = chs;
        step.leftFreq = only->freq;
        step.rightChars = "-";
        step.rightFreq = 0;
        step.mergedFreq = only->freq;
        m_steps.append(step);
        return;
    }

    while (!heap.empty()) {
        HuffmanNode* n1 = heap.top(); heap.pop();
        if (heap.empty()) {
            // 最后一个就是根
            m_root = n1;
            break;
        }
        HuffmanNode* n2 = heap.top(); heap.pop();
        HuffmanNode* merged = new HuffmanNode(QChar(), n1->freq + n2->freq, n1, n2);

        // 记录这一步合并的信息
        MergeStep step;
        QString charsLeft, charsRight;
        collectChars(n1, charsLeft);
        collectChars(n2, charsRight);
        step.leftChars = charsLeft;
        step.leftFreq = n1->freq;
        step.rightChars = charsRight;
        step.rightFreq = n2->freq;
        step.mergedFreq = merged->freq;
        m_steps.append(step);

        heap.push(merged);

        if (heap.size() == 1) {
            // 下次循环会把 merged 弹出当作 n1 并结束
        }
    }
}

void HuffmanCodec::buildCodesRec(HuffmanNode* node,
                                 const QString& prefix,
                                 QHash<QChar, QString>& out) const
{
    if (!node)
        return;

    if (!node->left && !node->right) {
        // 叶子节点
        QString code = prefix;
        if (code.isEmpty())
            code = "0";
        out[node->ch] = code;
        return;
    }
    buildCodesRec(node->left, prefix + "0", out);
    buildCodesRec(node->right, prefix + "1", out);
}

QHash<QChar, QString> HuffmanCodec::buildCodes() const
{
    QHash<QChar, QString> codes;
    buildCodesRec(m_root, QString(), codes);
    return codes;
}

QString HuffmanCodec::encode(const QString& text) const
{
    if (!m_root)
        return QString();

    QHash<QChar, QString> codes = buildCodes();
    QString bits;
    bits.reserve(text.size() * 8);
    for (QChar c : text) {
        bits += codes.value(c);
    }
    return bits;
}

QString HuffmanCodec::decode(const QString& bits, bool* ok) const
{
    if (ok) *ok = false;
    if (!m_root)
        return QString();

    QString result;
    HuffmanNode* node = m_root;

    for (QChar b : bits) {
        if (b == '0') {
            node = node->left;
        } else if (b == '1') {
            node = node->right;
        } else {
            return QString(); // 非法字符
        }

        if (!node) {
            return QString(); // 非法路径
        }

        if (!node->left && !node->right) {
            result.append(node->ch);
            node = m_root;
        }
    }

    if (ok) *ok = true;
    return result;
}
