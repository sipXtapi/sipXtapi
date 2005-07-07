<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <table width="560" border="0">
			<tr>
				<td width="15"><img src="images/spacer.gif" width="15"/></td>
				<td width="100%">
					<div class="notetextdrk"><b>Call Handling Settings Overview</b><br/><br/></div>
					<table border="0" cellpadding="4" cellspacing="1" class="bglist" width="480">
						<tr>
							<th align="center" class="tableheader">Feature</th>
							<th align="center" class="tableheader">Current Setting</th>
						</tr>
						<tr>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>Incoming Call Alert</td>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>
								<xsl:choose>
									<xsl:when test="/details/configurationset/PROFILE/PHONESET_RINGER">
										<xsl:choose>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_RINGER/PHONESET_RINGER = 'BOTH'">
												<xsl:text>Audible and Visual</xsl:text>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_RINGER/PHONESET_RINGER = 'AUDIBLE'">
												<xsl:text>Audible</xsl:text>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_RINGER/PHONESET_RINGER = 'VISUAL'">
												<xsl:text>Visual</xsl:text>
											</xsl:when>
											<xsl:otherwise>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:text>Not Set</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
						<tr>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>Forward All Calls</td>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>
								<xsl:choose>
									<xsl:when test="/details/configurationset/PROFILE/PHONESET_FORWARD_UNCONDITIONAL">
										<xsl:choose>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_FORWARD_UNCONDITIONAL/PHONESET_FORWARD_UNCONDITIONAL = 'ENABLE'">
												<xsl:value-of select="/details/configurationset/PROFILE/SIP_FORWARD_UNCONDITIONAL/SIP_FORWARD_UNCONDITIONAL"/>
											</xsl:when>
											<xsl:otherwise>
												<xsl:text>Disabled</xsl:text>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:text>Not Set</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
						<tr>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>Forward On No Answer</td>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>
								<xsl:choose>
									<xsl:when test="/details/configurationset/PROFILE/PHONESET_AVAILABLE_BEHAVIOR">
										<xsl:choose>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_AVAILABLE_BEHAVIOR/PHONESET_AVAILABLE_BEHAVIOR = 'FORWARD_ON_NO_ANSWER'">
												<xsl:value-of select="/details/configurationset/PROFILE/SIP_FORWARD_ON_NO_ANSWER/SIP_FORWARD_ON_NO_ANSWER"/>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_AVAILABLE_BEHAVIOR/PHONESET_AVAILABLE_BEHAVIOR = 'RING'">
												<xsl:text>Disabled</xsl:text>
											</xsl:when>
											<xsl:otherwise>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:text>Not Set</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
						<tr>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>Forward On Busy</td>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>
								<xsl:choose>
									<xsl:when test="/details/configurationset/PROFILE/PHONESET_BUSY_BEHAVIOR">
										<xsl:choose>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_BUSY_BEHAVIOR/PHONESET_BUSY_BEHAVIOR = 'FORWARD'">
												<xsl:value-of select="/details/configurationset/PROFILE/SIP_FORWARD_ON_BUSY/SIP_FORWARD_ON_BUSY"/>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_BUSY_BEHAVIOR/PHONESET_BUSY_BEHAVIOR = 'BUSY'">
												<xsl:text>Busy</xsl:text>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_BUSY_BEHAVIOR/PHONESET_BUSY_BEHAVIOR = 'QUEUE'">
												<xsl:text>Queue</xsl:text>
											</xsl:when>
											<xsl:otherwise>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:text>Not Set</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
						<tr>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>Call Waiting</td>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>
								<xsl:choose>
									<xsl:when test="/details/configurationset/PROFILE/PHONESET_CALL_WAITING_BEHAVIOR">
										<xsl:choose>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_CALL_WAITING_BEHAVIOR/PHONESET_CALL_WAITING_BEHAVIOR = 'ALERT'">
												<xsl:text>Alert</xsl:text>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_CALL_WAITING_BEHAVIOR/PHONESET_CALL_WAITING_BEHAVIOR = 'BUSY'">
												<xsl:text>Busy</xsl:text>
											</xsl:when>
											<xsl:otherwise>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:text>Not Set</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
						<tr>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>Do Not Disturb</td>
							<td class="tablecontents"><img src="images/spacer.gif" width="15"/>
								<xsl:choose>
									<xsl:when test="/details/configurationset/PROFILE/PHONESET_DND">
										<xsl:choose>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_DND/PHONESET_DND = 'ENABLE'">
												<xsl:text>Enabled</xsl:text>
											</xsl:when>
											<xsl:when test="/details/configurationset/PROFILE/PHONESET_DND/PHONESET_DND = 'DISABLE'">
												<xsl:text>Disabled</xsl:text>
											</xsl:when>
											<xsl:otherwise>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:text>Not Set</xsl:text>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
					</table>
				</td>
				</tr>
			</table>
    </xsl:template>
</xsl:stylesheet>

