<?xml version="1.0" encoding="utf-8"?>
<test>
    <description>2D unsteady FRDG advection MODIFIED, P=3, Dirichlet bcs, deformed elements</description>
    <executable>ADRSolver</executable>
    <parameters>Test_Advection2D_dirichlet_deformed_MODIFIED_10x10.xml</parameters>
    <files>
        <file description="Session File">Test_Advection2D_dirichlet_deformed_MODIFIED_10x10.xml</file>
    </files>
    <metrics>
        <metric type="L2" id="1">
            <value variable="u" tolerance="1e-12"> 0.000376742 </value>
        </metric>
        <metric type="Linf" id="2">
            <value variable="u" tolerance="1e-12"> 0.00303025 </value>
        </metric>
    </metrics>
</test>