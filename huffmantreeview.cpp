#ifndef HUFFMANTREEVIEW_H
#define HUFFMANTREEVIEW_H

#include <QGraphicsView>
#include "huffman.h"

// 可缩放、可拖动的 Huffman 树视图
class HuffmanTreeView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit HuffmanTreeView(QWidget* parent = nullptr);

    // 设置 Huffman 树根结点，视图会自动重建
    void setRoot(HuffmanNode* root);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QGraphicsScene* m_scene;
    HuffmanNode*    m_root;

    bool    m_panning;
    QPoint  m_lastPos;

    // 重建整棵树的图形
    void rebuildScene();

    // 第一遍：给每个叶子编号（从左到右），并记录最大深度
    void assignLeafIndex(HuffmanNode* node,
                         QMap<HuffmanNode*, int>& indexMap,
                         int& currentIndex,
                         int depth,
                         int& maxDepth);

    // 根据节点、深度、叶子 index 计算节点坐标
    QPointF nodePos(HuffmanNode* node,
                    const QMap<HuffmanNode*, int>& indexMap,
                    int depth,
                    int maxDepth,
                    int leafCount,
                    const QRectF& area);

    // 递归向 scene 中添加节点和连线
    void addItemsRecursive(HuffmanNode* node,
                           const QMap<HuffmanNode*, int>& indexMap,
                           int depth,
                           int maxDepth,
                           int leafCount,
                           const QRectF& area);
};

#endif // HUFFMANTREEVIEW_H
