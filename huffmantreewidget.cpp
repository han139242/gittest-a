#include "huffmantreewidget.h"

#include <QGraphicsScene> // 场景
#include <QGraphicsRectItem> // 矩形项
#include <QGraphicsTextItem> // 文字项
#include <QWheelEvent> // 滚轮事件
#include <QFontMetrics> // 字体测量
#include <QPalette> // 调色板
#include <QtMath> // 数学函数
#include <QTimer>

HuffmanTreeWidget::HuffmanTreeWidget(QWidget* parent)
    : QGraphicsView(parent),
    m_scene(new QGraphicsScene(this)),
    m_root(nullptr),
    m_levelHeight(80.0), // 层高80
    m_hSpacing(25.0), //间距20
    m_minNodeWidth(36.0), // 圆直径36
    m_animationTimer(new QTimer(this)),
    m_drawIndex(0)
{
    setScene(m_scene);

    // 背景颜色
    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor(245, 248, 255));
    setPalette(pal);
    setBackgroundBrush(QBrush(QColor(245, 248, 255)));

    setRenderHint(QPainter::Antialiasing, true);

    // 拖动 & 缩放设置
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    setMinimumHeight(300); // 增加最小高度

    connect(m_animationTimer, &QTimer::timeout, this, &HuffmanTreeWidget::onAnimationStep);
}

void HuffmanTreeWidget::setRoot(HuffmanNode* root)
{
    m_root = root;
    rebuildScene(); // 只要树变了，就全部重画
}

void HuffmanTreeWidget::rebuildScene()
{
    // 停止之前的动画
    m_animationTimer->stop();
    m_nodesToDraw.clear();
    m_drawIndex = 0;

    // 清空旧图
    m_scene->clear();
    m_layout.clear();
    m_nodeItems.clear();
    m_edgeItems.clear();

    if (!m_root) {
        QGraphicsTextItem* textItem = m_scene->addText(tr("尚未构造 Huffman 树"));
        QFont f = textItem->font();
        f.setPointSize(10);
        textItem->setFont(f);
        textItem->setDefaultTextColor(Qt::gray);
        QRectF br = textItem->boundingRect();
        textItem->setPos(-br.width() / 2.0, -br.height() / 2.0);
        resetTransform(); // 重置缩放
        fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        return;
    }

    // 1. 递归计算每个节点的布局（中心坐标 & 子树宽）
    // 从 x = 0 开始布，layoutSubtree 会把左右子树错开，避免重叠
    LayoutInfo info = layoutSubtree(m_root, 0, 0.0);

    Q_UNUSED(info); //抑制未使用参数的编译警告

    // 2. 准备动画数据
    collectNodesForAnimation(m_root);
    
    // 3. 启动动画，每 500ms 画一个节点（或者根据节点数量动态调整速度）
    m_animationTimer->start(300); 
}

void HuffmanTreeWidget::collectNodesForAnimation(HuffmanNode* node)
{
    if (!node) return;
    // 后序遍历：先画子节点，再画父节点，模拟合并过程
    collectNodesForAnimation(node->left);
    collectNodesForAnimation(node->right);
    m_nodesToDraw.append(node);
}

void HuffmanTreeWidget::onAnimationStep()
{
    if (m_drawIndex >= m_nodesToDraw.size()) {
        m_animationTimer->stop();
        // 动画结束后适配视图
        QRectF bounds = m_scene->itemsBoundingRect();
        if (!bounds.isNull()) {
            bounds.adjust(-40, -40, 40, 40);
            fitInView(bounds, Qt::KeepAspectRatio);
        }
        return;
    }

    HuffmanNode* node = m_nodesToDraw[m_drawIndex];
    drawNode(node);
    m_drawIndex++;
    
    // 每次绘制后稍微调整视图以确保新节点可见（可选）
    // ensureVisible(m_nodeItems[node]);
}

void HuffmanTreeWidget::drawNode(HuffmanNode* node)
{
    if (!m_layout.contains(node)) return;

    LayoutInfo info = m_layout[node];
    QPointF center = info.pos;

    QFont font;
    font.setPointSize(9);
    QFontMetrics fm(font);

    // 1. 画连线（连接到子节点）
    auto drawEdge = [&](HuffmanNode* childNode) {
        if (!childNode) return;
        if (!m_layout.contains(childNode)) return;

        QPointF cPos = m_layout.value(childNode).pos;
        double nodeH = 24.0;
        QPointF pBottom(center.x(), center.y() + nodeH / 2.0);
        QPointF cTop   (cPos.x(), cPos.y() - nodeH / 2.0);

        QGraphicsLineItem* line = m_scene->addLine(QLineF(pBottom, cTop),
                         QPen(QColor(180, 180, 200), 1.4));
        line->setZValue(0);
        m_edgeItems[childNode] = line; // 记录连线，key是子节点
    };

    drawEdge(node->left);
    drawEdge(node->right);

    // 2. 画节点
    QString text;
    if (!node->left && !node->right) {
        QChar c = node->ch;
        QString showChar;
        if (c == ' ')      showChar = "[sp]";
        else if (c == '\n') showChar = "[\\n]";
        else if (c == '\t') showChar = "[\\t]";
        else               showChar = c;
        text = QString("%1\n%2").arg(showChar).arg(node->freq);
    } else {
        text = QString::number(node->freq);
    }

    QRectF textRect = fm.boundingRect(QRect(0, 0, 1000, 1000),
                                      Qt::AlignCenter | Qt::TextWordWrap,
                                      text);
    double marginW = 10.0;
    double marginH = 4.0;
    double rectW = qMax(m_minNodeWidth, textRect.width() + marginW * 2);
    double rectH = textRect.height() + marginH * 2;

    QRectF nodeRect(center.x() - rectW / 2.0,
                    center.y() - rectH / 2.0,
                    rectW, rectH);

    QGraphicsRectItem* box = m_scene->addRect(
        nodeRect,
        Qt::NoPen,
        QBrush(QColor(110, 180, 255, 220)));
    box->setZValue(1);
    m_nodeItems[node] = box; // 记录节点图形项

    QGraphicsTextItem* textItem = m_scene->addText(text, font);
    textItem->setDefaultTextColor(Qt::white);
    QRectF br = textItem->boundingRect();
    textItem->setPos(center.x() - br.width()/2.0,
                     center.y() - br.height()/2.0);
    textItem->setZValue(2);
}

void HuffmanTreeWidget::buildGraphics()
{
    // 此函数不再直接使用，逻辑移至 drawNode 和 onAnimationStep
    // 但为了兼容性保留空实现或直接调用全量绘制
}

void HuffmanTreeWidget::highlightPath(const QString& code)
{
    // 1. 重置所有节点和连线颜色
    for (auto item : m_nodeItems) {
        item->setBrush(QBrush(QColor(110, 180, 255, 220)));
    }
    for (auto item : m_edgeItems) {
        item->setPen(QPen(QColor(180, 180, 200), 1.4));
    }

    if (!m_root || code.isEmpty()) return;

    // 2. 遍历路径并高亮
    HuffmanNode* curr = m_root;
    
    // 高亮根节点
    if (m_nodeItems.contains(curr)) {
        m_nodeItems[curr]->setBrush(QBrush(QColor(255, 100, 100, 220))); // 红色
    }

    for (QChar c : code) {
        HuffmanNode* next = nullptr;
        if (c == '0') next = curr->left;
        else if (c == '1') next = curr->right;

        if (next) {
            // 高亮连线
            if (m_edgeItems.contains(next)) {
                m_edgeItems[next]->setPen(QPen(QColor(255, 0, 0), 2.5));
            }
            // 高亮节点
            if (m_nodeItems.contains(next)) {
                m_nodeItems[next]->setBrush(QBrush(QColor(255, 100, 100, 220)));
            }
            curr = next;
        } else {
            break; 
        }
    }
}

HuffmanTreeWidget::LayoutInfo
HuffmanTreeWidget::layoutSubtree(HuffmanNode* node, int depth, double x)
{
    LayoutInfo info;
    if (!node) {
        info.pos = QPointF(x, depth * m_levelHeight);
        info.width = m_minNodeWidth;
        return info;
    }

    // 叶子：直接返回最小宽度
    if (!node->left && !node->right) {
        info.width = m_minNodeWidth;
        info.pos = QPointF(x, depth * m_levelHeight);
        m_layout[node] = info;
        return info;
    }

    // 有孩子节点：分别计算左右子树的布局
    LayoutInfo leftInfo;
    LayoutInfo rightInfo;

    double leftWidth = 0.0;
    double rightWidth = 0.0;

    if (node->left) {
        leftInfo = layoutSubtree(node->left, depth + 1, x);
        leftWidth = leftInfo.width;
    }

    if (node->right) {
        // 右子树的起始 x = 左子树右边 + 间距
        double rightStartX = x + leftWidth + m_hSpacing;
        rightInfo = layoutSubtree(node->right, depth + 1, rightStartX);
        rightWidth = rightInfo.width;
    }

    if (!node->left && node->right) {
        // 只有右子树
        info.width = qMax(rightWidth, m_minNodeWidth);
        info.pos = QPointF(rightInfo.pos.x(), depth * m_levelHeight);
    } else if (node->left && !node->right) {
        // 只有左子树
        info.width = qMax(leftWidth, m_minNodeWidth);
        info.pos = QPointF(leftInfo.pos.x(), depth * m_levelHeight);
    } else {
        // 左右都存在：父节点放在两个子树中心之间
        double subtreeWidth = leftWidth + m_hSpacing + rightWidth;
        info.width = qMax(subtreeWidth, m_minNodeWidth);
        double centerX = leftInfo.pos.x() + leftWidth / 2.0 + m_hSpacing / 2.0 + rightWidth / 2.0;
        info.pos = QPointF(centerX, depth * m_levelHeight);
    }

    m_layout[node] = info;
    return info;
}

void HuffmanTreeWidget::wheelEvent(QWheelEvent* event)
{
    const double factor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(factor, factor);
    } else {
        scale(1.0 / factor, 1.0 / factor);
    }
    event->accept();
}
