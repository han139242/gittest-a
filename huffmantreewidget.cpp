#include "huffmantreewidget.h"

#include <QGraphicsScene> // 场景
#include <QGraphicsRectItem> // 矩形项
#include <QGraphicsTextItem> // 文字项
#include <QWheelEvent> // 滚轮事件
#include <QFontMetrics> // 字体测量
#include <QPalette> // 调色板
#include <QtMath> // 数学函数

HuffmanTreeWidget::HuffmanTreeWidget(QWidget* parent)
    : QGraphicsView(parent),
    m_scene(new QGraphicsScene(this)),
    m_root(nullptr),
    m_levelHeight(80.0), // 层高80
    m_hSpacing(25.0), //间距20
    m_minNodeWidth(36.0) // 圆直径36
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

    setMinimumHeight(200);
}

void HuffmanTreeWidget::setRoot(HuffmanNode* root)
{
    m_root = root;
    rebuildScene(); // 只要树变了，就全部重画
}

void HuffmanTreeWidget::rebuildScene()
{
    // 清空旧图
    m_scene->clear();
    m_layout.clear();

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

    // 2. 根据布局在 scene 中绘制节点和连线
    buildGraphics();

    // 3. 适配当前窗口
    resetTransform();
    QRectF bounds = m_scene->itemsBoundingRect();
    if (!bounds.isNull()) {
        bounds.adjust(-40, -40, 40, 40);
        fitInView(bounds, Qt::KeepAspectRatio);
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

void HuffmanTreeWidget::buildGraphics()
{
    if (!m_root)
        return;

    QFont font;
    font.setPointSize(9);
    QFontMetrics fm(font);

    // 1. 先画线（保证在线的下面）
    for (auto it = m_layout.constBegin(); it != m_layout.constEnd(); ++it) {
        HuffmanNode* node = it.key();
        QPointF pPos = it.value().pos;

        auto drawEdge = [&](HuffmanNode* childNode) {
            if (!childNode) return;
            if (!m_layout.contains(childNode)) return;

            QPointF cPos = m_layout.value(childNode).pos;

            // 计算从父到子矩形边缘的连线（避免穿过节点中心）
            // 这里假定矩形宽度稍后计算；先用一个估算高度 = 24
            double nodeH = 24.0;

            QPointF pBottom(pPos.x(), pPos.y() + nodeH / 2.0);
            QPointF cTop   (cPos.x(), cPos.y() - nodeH / 2.0);

            m_scene->addLine(QLineF(pBottom, cTop),
                             QPen(QColor(180, 180, 200), 1.4));
        };

        drawEdge(node->left);
        drawEdge(node->right);
    }

    // 2. 再画节点和文字
    for (auto it = m_layout.constBegin(); it != m_layout.constEnd(); ++it) {
        HuffmanNode* node = it.key();
        QPointF center = it.value().pos;

        // 准备显示文字
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

        // 计算文本实际尺寸，给节点留足空间
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

        // 更新：为后续画线修正高度，避免线条插进矩形内部
        // 这里直接在 layout 中更新高度信息方便。
        // 注意：线条部分之前用了固定 24，这里可以稍微大些也没关系，不会看出明显穿帮。
        // 若要更精确，可以把高度也存进 LayoutInfo 并用它画线。
        // 简化起见，这里仅根据视觉效果设置一个稍大的值即可。
        // （如需更严谨我可以再给你一版带高度的 LayoutInfo）
        Q_UNUSED(nodeRect);

        QGraphicsRectItem* box = m_scene->addRect(
            nodeRect,
            Qt::NoPen,
            QBrush(QColor(110, 180, 255, 220)));
        box->setZValue(1);

        QGraphicsTextItem* textItem = m_scene->addText(text, font);
        textItem->setDefaultTextColor(Qt::white);
        QRectF br = textItem->boundingRect();
        textItem->setPos(center.x() - br.width()/2.0,
                         center.y() - br.height()/2.0);
        textItem->setZValue(2);
    }
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
