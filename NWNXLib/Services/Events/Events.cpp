#include "Services/Events/Events.hpp"
#include "API/CExoString.hpp"
#include "API/CGameEffect.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>

namespace NWNXLib {

namespace Services {

Events::Events()
{
}

Events::~Events()
{
}

Events::RegistrationToken Events::RegisterEvent(const std::string& pluginName, const std::string& eventName, FunctionCallback&& cb)
{
    EventList& events = m_eventMap[pluginName];

    auto event = std::find_if(std::begin(events), std::end(events),
        [&eventName](const std::unique_ptr<EventDataInternal>& data) -> bool
        {
            return data->m_data.m_eventName == eventName;
        }
    );

    if (event != std::end(events))
    {
        throw std::runtime_error("Tried to register an event twice with the same name.");
    }

    EventData eventData = { pluginName, eventName };
    auto eventDataInternal = std::make_unique<EventDataInternal>();
    eventDataInternal->m_data = eventData;
    eventDataInternal->m_callback = std::forward<FunctionCallback>(cb);
    events.emplace_back(std::move(eventDataInternal));

    return { std::move(eventData) };
}

void Events::ClearEvent(RegistrationToken&& token)
{
    auto pluginEvents = m_eventMap.find(token.m_data.m_pluginName);

    if (pluginEvents == std::end(m_eventMap))
    {
        throw std::runtime_error("Invalid or duplicate event registration token.");
    }

    EventList& eventsList = pluginEvents->second;

    auto event = std::find_if(std::begin(eventsList), std::end(eventsList),
        [&token](const std::unique_ptr<EventDataInternal>& data) -> bool
        {
            return data->m_data.m_eventName == token.m_data.m_eventName;
        }
    );

    if (event == std::end(eventsList))
    {
        throw std::runtime_error("Invalid or duplicate event registration token.");
    }

    eventsList.erase(event);
}


EventsProxy::EventsProxy(Events& events, std::string pluginName)
    : ServiceProxy<Events>(events), m_pluginName(pluginName)
{
}

EventsProxy::~EventsProxy()
{
    for (Events::RegistrationToken& token : m_registrationTokens)
    {
        m_proxyBase.ClearEvent(std::move(token));
    }
}

void EventsProxy::RegisterEvent(const std::string& eventName, Events::FunctionCallback&& cb)
{
    m_registrationTokens.push_back(m_proxyBase.RegisterEvent(m_pluginName, eventName, std::forward<Events::FunctionCallback>(cb)));
}

void EventsProxy::ClearEvent(const std::string& eventName)
{
    auto token = std::find_if(std::begin(m_registrationTokens), std::end(m_registrationTokens),
        [this, &eventName](const Events::RegistrationToken& check)
        {
            return check.m_data.m_pluginName == m_pluginName && check.m_data.m_eventName == eventName;
        }
    );

    if (token == std::end(m_registrationTokens))
    {
        throw std::runtime_error("Tried to clear unrecognised event.");
    }

    Events::RegistrationToken concreteToken = std::move(*token);
    m_registrationTokens.erase(token);
    m_proxyBase.ClearEvent(std::move(concreteToken));
}



template<> Maybe<int32_t>&              Events::Argument::Get<int32_t>()             { return Int; }
template<> Maybe<float>&                Events::Argument::Get<float>()               { return Float; }
template<> Maybe<API::Types::ObjectID>& Events::Argument::Get<API::Types::ObjectID>(){ return Object; }
template<> Maybe<std::string>&          Events::Argument::Get<std::string>()         { return String; }
template<> Maybe<API::CGameEffect>&     Events::Argument::Get<API::CGameEffect>()    { return Effect; }

std::string Events::Argument::toString()
{
    if (Int)    return std::to_string(*Int);
    if (Float)  return std::to_string(*Float);
    if (Object) return Utils::ObjectIDToString(*Object);
    if (String) return *String;
    if (Effect) return std::string("EffectID:") + std::to_string(Effect->m_nID);

    return std::string("");
}

}

}
