import me.hd.wauxv.plugin.api.PluginMethod;
import me.hd.wauxv.data.bean.MsgInfo;

import java.util.ArrayList;
import java.util.List;

public void sendText(String talker, String content) {
	PluginMethod.sendText(talker, content);
}

public void kickGroup(String qun, List List) {
	PluginMethod.sendDelChatroomMember(qun, List);
}
// 收到消息回调
public void onHandleMsg(MsgInfo data) {
	String text = data.content;
	String qun = data.talker;
	String wxid = data.sendTalker;
	if (data.isSend() && data.isGroupChatMessage()) {
        String title = getElementContent(text, "title");
		if (data.isQuote() && title.equals("/ban")) {
			String Kick = getElementContent(text, "refermsg", "chatusr");
            List delMemberList = new ArrayList();
            delMemberList.add(Kick);
            kickGroup(qun, delMemberList);
            sendText(qun, Kick + "\n已被踢出此群");
		}
	}
}

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.xml.sax.InputSource;
import org.w3c.dom.*;

public String getElementContent(String xmlString, String tagName) {
	try {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder builder = factory.newDocumentBuilder();
		ByteArrayInputStream input = new ByteArrayInputStream(xmlString.getBytes("UTF-8"));
		Document document = builder.parse(input);
		NodeList elements = document.getElementsByTagName(tagName);
		if (elements.getLength() > 0) {
			return elements.item(0).getTextContent();
		}
	} catch (Exception e) {
		return null;
	}
	return null;
}
public String getElementAttribute(String xmlString, String tagName, String attributeName) {
	try {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder builder = factory.newDocumentBuilder();
		Document document = builder.parse(new ByteArrayInputStream(xmlString.getBytes("UTF-8")));
		Element element = (Element) document.getElementsByTagName(tagName).item(0);
		if (element != null) {
			return element.getAttribute(attributeName);
		}
	} catch (Exception e) {
		return null;
	}
	return null;
}
public String getElementContent(String xmlString, String elementName, String tagName) {
	try {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder builder = factory.newDocumentBuilder();
		Document document = builder.parse(new InputSource(new StringReader(xmlString)));
		NodeList referMsgList = document.getElementsByTagName(elementName);
		if (referMsgList.getLength() > 0) {
			Node referMsgNode = referMsgList.item(0);
			NodeList contentList = referMsgNode.getChildNodes();
			for (int i = 0; i < contentList.getLength(); i++) {
				Node contentNode = contentList.item(i);
				if (contentNode.getNodeName().equalsIgnoreCase(tagName)) {
					return contentNode.getTextContent();
				}
			}
		}
	} catch (Exception e) {
		return null;
	}
	return null;
}