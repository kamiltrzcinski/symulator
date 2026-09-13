import math

def generate_cpp():
    return """
    // ----------------------------------------------------
    // Obliczenia wektora dla odnogi
    // ----------------------------------------------------
    qreal dx = endX - startX;
    qreal dy = endY - startY;
    qreal L = std::hypot(dx, dy);
    qreal ux = dx / L;
    qreal uy = dy / L;

    // Przerwa u nasady (iglica vs tor) - tor ma grubość 4 (promień 2). Dodajemy 1px przerwy = 3px od osi, ale pod kątem to ok. 3.5.
    qreal rootGap = 3.5; 
    // Przerwa na środku rozjazdu (pomiędzy górnym a dolnym rozjazdem)
    qreal midGap = 1.0; 
    
    QPointF p1(startX + ux * rootGap, startY + uy * rootGap);
    QPointF p2(startX + ux * (L/2 - midGap), startY + uy * (L/2 - midGap));
    
    QPointF p3(startX + ux * (L/2 + midGap), startY + uy * (L/2 + midGap));
    QPointF p4(endX - ux * rootGap, endY - uy * rootGap);

    painter->drawLine(p1, p2);
    painter->drawLine(p3, p4);
"""
print(generate_cpp())
