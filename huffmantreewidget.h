#ifndef HUFFMANTREEWIDGET_H
#define HUFFMANTREEWIDGET_H

#include <QGraphicsView> // 视图框架基类
#include <QMap> //用于存储布局信息
#include <QWidget>
#include <QTimer>
#include "huffman.h" //用于存储布局信息

/*
  使用 QGraphicsView的 Huffman 树可视化控件
  - 支持缩放、拖动
  - 自适应节点宽度，避免文字被截断
  - 根据真实子树宽度布局，避免左右子树交叉/重叠
  - 支持动态生成动画
  - 支持编码路径高亮
 */
class HuffmanTreeWidget : public QGraphicsView
{
    Q_OBJECT
public:
    // 构造函数 explicit 防止隐式转换
    explicit HuffmanTreeWidget(QWidget* parent = nullptr);

    // 传入 Huffman 树根节点,传入树根，然后绘制
    void setRoot(HuffmanNode* root);

    // 高亮显示指定编码对应的路径
    void highlightPath(const QString& code);

protected:
    // 重写滚轮事件：实现缩放功能
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void onAnimationStep();

private:
    QGraphicsScene* m_scene; // 所有画出来的圆圈、线条都放在这里
    HuffmanNode* m_root; // 树根指针

    //(布局信息)
    struct LayoutInfo {
        QPointF pos;    // 节点的中心坐标 (x, y)
        double width;   // 这棵子树的总宽度
    };

    // 清空旧图,根据 Huffman 树重建整个场景
    void rebuildScene();

    // 递归布局，递归计算每个节点的坐标，返回该子树的 LayoutInfo(布局信息)--只计算
    LayoutInfo layoutSubtree(HuffmanNode* node, int depth, double x);

    // 根据计算好的坐标，往场景里加圆圈和线条--和计算好的配合着用
    void buildGraphics();

    // 存储每个节点的布局信息
    QMap<HuffmanNode*, LayoutInfo> m_layout;
    // 存储节点对应的图形项，用于高亮
    QMap<HuffmanNode*, QGraphicsRectItem*> m_nodeItems;
    QMap<HuffmanNode*, QGraphicsLineItem*> m_edgeItems; // 存储连线，key是子节点

    // 基本参数
    double m_levelHeight;      // 层间垂直距离
    double m_hSpacing;         // 不同子树之间的最小水平间距
    double m_minNodeWidth;     // 最小节点宽度

    // 动画相关
    QTimer* m_animationTimer;
    QList<HuffmanNode*> m_nodesToDraw; // 按层序遍历或其他顺序存储待绘制节点
    int m_drawIndex;
    void collectNodesForAnimation(HuffmanNode* node);
    void drawNode(HuffmanNode* node);
};

#endif // HUFFMANTREEWIDGET_H
